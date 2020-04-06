#include <catch2/catch.hpp>
#include "Utils/Writer.hpp"
#include <filesystem>
#include <fstream>
#include <cstdint>

#include <string>
#pragma warning( disable : 6237 6319 )

namespace fs = std::filesystem;

const std::string path = ".writer.test.tmp";

SCENARIO("Writer creates new file", "[Writer]")
{
	GIVEN("a file that doesn't exist")
	{
		fs::remove(path);
		REQUIRE_FALSE(fs::exists(path));

		WHEN("Writer is created")
		{
			Writer writer(path);

			THEN("File is created")
			{
				CHECK(fs::exists(path));
			}
		}
	}

	GIVEN("a file that exists")
	{
		std::ofstream(path) << "abc";
		REQUIRE(fs::exists(path));

		WHEN("Writer is created")
		{
			Writer writer(path);

			THEN("File is overriden")
			{
				CHECK(fs::exists(path));
				CHECK(fs::is_empty(path));
			}
		}
	}
	fs::remove(path);
}

#include <iostream>

SCENARIO("Writer writes number to a file", "[Writer]")
{
	GIVEN("a number that can be represented on 5 bits")
	{
		Writer writer(path);
		uint64_t n = 23; // 0001 0111 = 0x17

		WHEN("number is written on 5 bits")
		{
			// number is saved as 1'0111|000 = 1011'1000 = 0xB8 (1 byte)
			//          number -> ^^^^^^|*** <- padding
			writer.write(n, 5);
			writer.flush();

			THEN("number is saved correctly")
			{
				std::ifstream file(path, std::ifstream::binary);
				unsigned char byte = file.get();
				int eof = file.get();

				CHECK(byte == 0xB8);
				CHECK(eof == EOF);
			}
		}

		WHEN("number is written on 8 bits")
		{
			// number is saved as 0001'0111 = 0x17 (no padding necessary)
			writer.write(n, 8);	
			writer.flush();

			THEN("number is saved correctly")
			{
				std::ifstream file(path, std::ifstream::binary);
				unsigned char byte = file.get();
				int eof = file.get();

				CHECK(byte == 0x17);
				CHECK(eof == EOF);
			}
		}

		WHEN("number is written on 11 bits")
		{
			// number is saved as 000'0001'0111|0'0000 = 0000'0010'1110'0000
			//          number -> ^^^^^^^^^^^^^|****** <- padding
			// first 11 bits represent `n` and then at the end 
			// of file there is a padding with zeroes to fill whole byte.
			writer.write(n, 11);
			writer.flush();

			THEN("number is saved correctly")
			{
				std::ifstream file(path, std::ifstream::binary);
				unsigned char firstByte = file.get();
				unsigned char secondByte = file.get();
				int eof = file.get();

				CHECK(firstByte == 0x02);
				CHECK(secondByte == 0xE0);
				CHECK(eof == EOF);
			}
		}
	}
	fs::remove(path);
}