#include <catch2/catch.hpp>
#include "LZWCoder.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#pragma warning( disable : 6237 6319 )

namespace fs = std::filesystem;

const std::string encodePath = ".encode.test.tmp";
const std::string decodePath = ".decode.test.tmp";

SCENARIO("Encoder creates a file", "[Encoder]")
{
	LZWCoder coder;

	GIVEN("a path to non-existent file")
	{
		fs::remove(decodePath);
		REQUIRE(fs::exists(decodePath) == false);
		WHEN("input file is encoded")
		{
			std::ofstream(encodePath) << "abc";
			coder.encode(encodePath, decodePath);

			THEN("output file is created")
			{
				CHECK(fs::exists(decodePath));
			}
		}
	}
	GIVEN("a path to existing file")
	{
		std::ofstream(decodePath) << "some data to override";
		WHEN("input file is encoded")
		{
			std::ofstream(encodePath) << "a";
			coder.encode(encodePath, decodePath);
			THEN("output file is overriden")
			{
				REQUIRE(fs::exists(decodePath));
				unsigned char byte = std::ifstream(decodePath, std::ifstream::binary).get();
				CHECK(byte != 's');
			}
		}
	}
}

SCENARIO("Encoding files")
{
	LZWCoder coder;
	
	
	GIVEN("a file with one letter to encode")
	{
		std::ofstream(encodePath) << "a"; // 'a' =  97 = 0x61
		fs::remove(decodePath);

		WHEN("file is encoded")
		{
			// letter 'a' is encoded on 9 bits as: 0'0000'0110'0000'0001 = [0] + 0x61
			// file must be padded with zeroes:    0'0000'0110'0000'0001|0000000
			//                      number 0x61 -> 0 ^^^^^^^^^^^^^^^^^^^|******* <- padding
			// result file contains bytes: 0000'0011'0000'0000'1000'0000 = 0x030080
			coder.encode(encodePath, decodePath); 

			THEN("output file contains that letter on 9 bits")
			{
				std::ifstream file(decodePath, std::ofstream::binary);
				unsigned char byte1 = file.get();
				unsigned char byte2 = file.get();
				unsigned char byte3 = file.get();
				int eof = file.get();

				CHECK(byte1 == 0x03);
				CHECK(byte2 == 0x00);
				CHECK(byte3 == 0x80);
				CHECK(eof == EOF);
			}
		}
	}
}