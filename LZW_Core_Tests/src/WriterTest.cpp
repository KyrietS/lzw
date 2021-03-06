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

		WHEN("number is written on 9 bits")
		{
			// number is saved as 0'0001'0111|000'0000 = 0000'1011'1000'0000 = 0x0B80
			writer.write(n, 9);
			writer.flush();
			THEN("number is saved correctly")
			{
				std::ifstream file(path, std::ifstream::binary);
				unsigned char byte1 = file.get();
				unsigned char byte2 = file.get();
				int eof = file.get();

				CHECK(byte1 == 0x0B);
				CHECK(byte2 == 0x80);
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

	GIVEN("letter 'a'")
	{
		Writer writer(path);
		uint64_t n = 97; // 'a' = 0x61 = 0110'0001

		WHEN("number is written on 9 bits")
		{
			// number is saved as 0'0110'0001|000'0000 = 0011'0000'1000'0000 = 0x3080
			writer.write(n, 9);
			writer.flush();
			THEN("number is saved correctly")
			{
				std::ifstream file(path, std::ifstream::binary);
				unsigned char byte1 = file.get();
				unsigned char byte2 = file.get();
				int eof = file.get();

				CHECK(byte1 == 0x30);
				CHECK(byte2 == 0x80);
				CHECK(eof == EOF);
			}
		}
	}
	fs::remove(path);
}

SCENARIO("Writer collects stats about written bytes", "[Writer][Stats]")
{
	WHEN("no data is written")
	{
		Writer writer(path);
		THEN("numbers' stats are zeros")
		{
			for (int i = 0; i <= 255; i++)
				REQUIRE(writer.stats[i] == 0);
		}
	}

	WHEN("number 7 is written on 1 byte")
	{
		Writer writer(path);
		writer.write(7, 8); // write '7' on 8 bits.
		writer.flush();

		THEN("number of 7s in stats is 1")
		{
			CHECK(writer.stats[6] == 0);
			CHECK(writer.stats[7] == 1);
			CHECK(writer.stats[8] == 0);
		}
	}

	WHEN("number 6 is written on 4 bits")
	{
		Writer writer(path);
		writer.write(6, 4); // write '6' on 4 bits
							// 0110 -> 0110'0000
							// number->^^^^|**** <- padding
		writer.flush();
		THEN("byte 0x60 occured once")
		{
			CHECK(writer.stats[0x60] == 1);
		}
	}

	WHEN("number 3 is written on 9 bits")
	{
		Writer writer(path);
		writer.write(3, 9); // write '3' on 9 bits
							// 0'0000'0011 -> 0'0000'0011|000'0000
							//      number -> ^^^^^^^^^^^|******** <- padding
							// result: 0000'0001'1000'0000 = 0x0180
		writer.flush();
		THEN("byte 0x01 and 0x80 occured once")
		{
			CHECK(writer.stats[0x01] == 1);
			CHECK(writer.stats[0x80] == 1);
		}
	}

	WHEN("number 0 is written on 9 bits")
	{
		Writer writer(path);
		writer.write(0, 9); // write '0' on 1 bit
							// 0'0000'0000 -> 0'0000'0000|000'0000
							//      number -> ^^^^^^^^^^^|******** <- padding
							// result: 0000'0000'0000'0000
		writer.flush();
		THEN("byte 0x00 occured twice")
		{
			CHECK(writer.stats[0x00] == 2);
		}
	}
	fs::remove(path);
}