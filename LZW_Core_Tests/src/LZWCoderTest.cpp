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

SCENARIO("Encoding small txt files", "[Encoder]")
{
	LZWCoder coder;
	
	GIVEN("a file with one letter to encode")
	{
		std::ofstream(encodePath) << "a"; // 'a' =  97 = 0x61
		fs::remove(decodePath);

		WHEN("file is encoded")
		{
			// letter 'a' is encoded on 8 bits as: 0110'0001 = 0x61
			coder.encode(encodePath, decodePath); 

			THEN("first letter is encoded on 8 bits")
			{
				std::ifstream file(decodePath, std::ofstream::binary);
				unsigned char byte1 = file.get();
				int eof = file.get();

				CHECK(byte1 == 0x61);
				CHECK(eof == EOF);
			}
		}
	}

	GIVEN("a file with two letters to encode")
	{
		std::ofstream(encodePath) << "ab"; // 0x6162
		fs::remove(decodePath);

		WHEN("file is encoded")
		{
			// letter 'a' is encoded on 8 bits as: 0110'0001 = 0x61

			// letter 'b' is encoded on 9 bits as: 0'0110'0010 = [0] + 0x62
			// file must be padded with zeroes:    0'0110'0010|000'0000
			//                      number 0x61 -> 0 ^^^^^^^^^|******* <- padding
			// result file contains bytes: 0110'0001'0011'0001'0000'0000 = 0x613100
			coder.encode(encodePath, decodePath);

			THEN("first letter is encoded on 8 bits")
			{
				std::ifstream file(decodePath, std::ofstream::binary);
				unsigned char byte1 = file.get();
				unsigned char byte2 = file.get();
				unsigned char byte3 = file.get();
				int eof = file.get();

				CHECK(byte1 == 0x61);
				CHECK(byte2 == 0x31);
				CHECK(byte3 == 0x00);
				CHECK(eof == EOF);
			}
		}
	}
	fs::remove(decodePath);
}

SCENARIO("Encoding binary files", "[Encoder]")
{
	LZWCoder coder;
	fs::remove(decodePath);

	GIVEN("a small binary")
	{
		std::ofstream file(encodePath, std::ofstream::binary);
		for (int byte = 0; byte < 255; byte++)
			file.put((char)byte);
		file.close();

		WHEN("file is encoded")
		{
			coder.encode(encodePath, decodePath);

			THEN("encoded file is created")
			{
				CHECK(fs::exists(decodePath));
			}
		}
	}
	fs::remove(decodePath);
}

SCENARIO("Decoding a file", "[Decoder]")
{
	LZWCoder coder;

	GIVEN("small text file")
	{
		std::ofstream(encodePath) << "abc";

		WHEN("file is encoded")
		{
			coder.encode(encodePath, decodePath);
			fs::remove(encodePath);

			THEN("file can be decoded")
			{
				coder.decode(decodePath, encodePath);
				REQUIRE(fs::exists(encodePath));
				std::string line;
				std::ifstream(encodePath) >> line;
				CHECK(line == "abc");
			}
		}
	}

	GIVEN("small binary file")
	{
		std::ofstream file(encodePath, std::ofstream::binary);
		for (int byte = 0; byte < 255; byte++)
			file.put((char)byte);
		file.close();

		WHEN("file is encoded")
		{
			coder.encode(encodePath, decodePath);
			fs::remove(encodePath);

			THEN("file can be decoded")
			{
				coder.decode(decodePath, encodePath);
				REQUIRE(fs::exists(encodePath));
				std::ifstream file(encodePath, std::ifstream::binary);
				for (int byte = 0; byte < 255; byte++)
					REQUIRE(file.get() == byte);
				CHECK(file.get() == EOF);
			}
		}
	}
}