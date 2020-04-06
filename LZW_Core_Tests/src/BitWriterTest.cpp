#include <catch2/catch.hpp>
#include <string>
#include <filesystem>
#include <fstream>
#include "Utils/BitWriter.hpp"

#pragma warning( disable : 6237 6319 )

namespace fs = std::filesystem;

SCENARIO("BitWriter creates and opens file", "[BitWriter]") {
	GIVEN("A file name") {
		std::string filename = "_file_1.test.tmp";
		fs::remove(filename);

		WHEN("File doesn't exist") {
			REQUIRE_FALSE(fs::exists(filename));

			THEN("BitWriter creates new file with given name") {
				BitWriter bw(filename);

				CHECK(fs::exists(filename));
			}
		}
		WHEN("File does exists") {
			std::ofstream someFile(filename);
			someFile << "Some data";
			someFile.close();

			REQUIRE(fs::exists(filename));

			THEN("BitWriter will overwrite it") {
				BitWriter writer(filename);
				CHECK(fs::exists(filename));
				CHECK(fs::is_empty(filename));
			}
		}
		fs::remove(filename);
	}
	GIVEN("An incorrect file name") {
		std::string incorrectFilename = GENERATE("", "@#$%^&*?", "\0a\0");

		WHEN("BitWriter tries to create it") {
			THEN("an error is thrown") {
				REQUIRE_THROWS(BitWriter(incorrectFilename));
				REQUIRE_FALSE(fs::exists(incorrectFilename));
			}
		}
	}
}

SCENARIO("BitWriter writes data to a file", "[BitWriter]") {
	std::string filename = "_file_1.test.tmp";
	fs::remove(filename);

	GIVEN("A BitWriter object") {
		BitWriter writer(filename);

		WHEN("one byte 'a' is written") {
			// 'a' = 0x61 = 0110'0001 <-- bits are read from left to right
			writer << 0 << 1 << 1 << 0;		// 0110
			writer << 0 << 0 << 0 << 1;     // 0001
			writer.flush(); // 0110 0001

			THEN("file contains that byte 'a'") {
				std::ifstream file(filename, std::ios_base::binary);
				char letterA;
				file.read(&letterA, 1);

				CHECK(letterA == 'a');
			}
		}
		WHEN("less than a byte is written") {
			writer << 1 << 0 << 1 << 1 << 0;
			writer.flush(); // 1011'0000

			THEN("file contains one byte padded with '0' bits") {
				std::ifstream file(filename, std::ios_base::binary);
				unsigned char byte = file.get();

				CHECK(byte == 0b1011'0000);
			}
		}
		WHEN("more than a byte is written") {
			writer << 1 << 1 << 0 << 0; // 1100
			writer << 0 << 1 << 1 << 0; // 0110
			writer << 1 << 1 << 1;      // 111
			writer.flush(); // 1100'0110'1110'0000
							//              ^^^^^^ <- padding

			THEN("file containes two bytes padded with '0' bits") {
				std::ifstream file(filename, std::ifstream::binary);
				unsigned char firstByte = file.get();
				unsigned char secondByte = file.get();

				CHECK(firstByte == 0b1100'0110);
				CHECK(secondByte == 0b1110'0000);
			}
		}
		WHEN("six same bits are written") {
			writer.writeN(1, 6);
			writer.flush(); // 1111'1100

			THEN("file contains six '1' bits") {
				std::ifstream file(filename, std::ios_base::binary);
				unsigned char byte = file.get();

				CHECK(byte == 0b1111'1100);
			}
		}
	}
	fs::remove(filename);
}
