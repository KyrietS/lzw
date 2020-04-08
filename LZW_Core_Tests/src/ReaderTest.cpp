#include <catch2/catch.hpp>
#include "Utils/Reader.hpp"
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <exception>

#include <string>
#pragma warning( disable : 6237 6319 )

namespace fs = std::filesystem;

const std::string path = ".reader.test.tmp";

SCENARIO("Reader opens a new file", "[Reader]")
{
	GIVEN("a file that doesn't exist")
	{
		fs::remove(path);
		REQUIRE_FALSE(fs::exists(path));

		WHEN("Reader is created")
		{
			THEN("an error is thrown")
			{
				CHECK_THROWS_AS(Reader(path), std::runtime_error);
			}
		}
	}

	GIVEN("a file that exists")
	{
		std::ofstream(path) << "abc";
		REQUIRE(fs::exists(path));
		Reader reader(path);

		WHEN("Reader is created")
		{
			THEN("Reader can read values from a file")
			{
				uint64_t n = reader.read(8);
				uint64_t n2 = reader.read(16);
			}
		}
		WHEN("Reader read all then content from a file")
		{
			REQUIRE(reader.eof() == false);
			reader.read(8); // a
			reader.read(8); // b
			reader.read(8); // c

			THEN("eof flag is set")
			{
				CHECK(reader.success() == true);
				CHECK(reader.eof() == true);
			}
		}
	}

	GIVEN("Reader with eof flag set to true")
	{
		std::ofstream(path) << "a";
		Reader reader(path);
		CHECK(reader.success() == true);
		CHECK(reader.eof() == false);
		reader.read(8);
		CHECK(reader.eof() == true);

		WHEN("Reader tries to read")
		{
			THEN("a zero number is returned")
			{
				CHECK(reader.read(1) == 0);
				CHECK(reader.read(4) == 0);
				CHECK(reader.read(8) == 0);
				CHECK(reader.read(32) == 0);

				AND_THEN("success flag is set to false")
				{
					CHECK(reader.success() == false);
				}
			}
		}
	}

	fs::remove(path);
}

#include <iostream>

SCENARIO("Reader reads numbers from a file", "[Reader]")
{
	GIVEN("a file with 1 byte")
	{
		std::ofstream(path) << '\xA7'; // 1010'0111
		Reader reader(path);

		WHEN("one 8-bit number is read")
		{
			uint64_t n = reader.read(8);

			THEN("number is correct")
			{
				CHECK(n == 0xA7);

				AND_THEN("eof flag is set")
				{
					CHECK(reader.success() == true);
					CHECK(reader.eof() == true);
				}
			}
		}

		WHEN("two 4-bit numbers are read")
		{
			uint64_t n1 = reader.read(4);
			uint64_t n2 = reader.read(4);

			THEN("numbers are correct")
			{
				CHECK(n1 == 0xA);
				CHECK(n2 == 0x7);

				AND_THEN("eof flag is set")
				{
					CHECK(reader.success() == true);
					CHECK(reader.eof() == true);
				}
			}
		}
		WHEN("one 3-bit number is read")
		{
			uint64_t n = reader.read(3); // 101

			THEN("number is correct")
			{
				CHECK(n == 0b101);

				AND_THEN("eof flag is not set")
				{
					CHECK(reader.success() == true);
					CHECK(reader.eof() == false);
				}
			}

		}
	}
	fs::remove(path);
}

SCENARIO("Reader collects stats about read bytes", "[Reader][Stats]")
{
	WHEN("no data is read")
	{
		std::ofstream(path, std::ofstream::binary) << '\x00';
		Reader reader(path);

		THEN("numbers' stats are zeros")
		{
			for (int i = 0; i <= 255; i++)
				REQUIRE(reader.stats[i] == 0);
		}
	}
	
	WHEN("number 7 is read on 1 byte")
	{
		std::ofstream(path, std::ofstream::binary) << '\x07';
		Reader reader(path);
		reader.read(8); // read '7' on 8 bits.


		THEN("number of 7s in stats is 1")
		{
			CHECK(reader.stats[6] == 0);
			CHECK(reader.stats[7] == 1);
			CHECK(reader.stats[8] == 0);
		}
	}

	WHEN("number 6 is read on 4 bits")
	{
		std::ofstream(path, std::ofstream::binary) << '\x62';
		Reader reader(path);
		reader.read(4);  // read '6' on 4 bits
						 // 0110 -> 0110'0010
		THEN("no byte occured yet")
		{
			for (int i = 0; i <= 255; i++)
				REQUIRE(reader.stats[i] == 0);

		}
		AND_WHEN("number 2 is read on 4 bits")
		{
			reader.read(4); // read '2' on 4 bits
			THEN("byte 0x62 occured once")
			{
				CHECK(reader.stats[0x62] == 1);
			}
		}
	}

	WHEN("number 3 is read on 9 bits")
	{
		std::ofstream(path, std::ofstream::binary) << '\x01' << '\x80';
		Reader reader(path);
		reader.read(9);		// file: 0000'0001'1000'0000 = 0x0180
							// read '3' on 9 bits
							// 0000'0001'1 -> 0000'0001'1|000'0000
							//      number -> ^^^^^^^^^^^|******** <- rest

		THEN("byte 0x01 occured once")
		{
			CHECK(reader.stats[0x01] == 1);
			CHECK(reader.stats[0x80] == 0);
		}
		AND_WHEN("number is read on 7 bits")
		{
			reader.read(7);
			THEN("byte 0x80 occured once")
			{
				CHECK(reader.stats[0x80] == 1);
			}
		}
	}

	WHEN("two numbers are read on two bytes")
	{
		std::ofstream(path, std::ofstream::binary) << '\xA7' << '\xA7';
		Reader reader(path);
		reader.read(7);
		reader.read(9);

		THEN("byte 0xA7 occured twice")
		{
			CHECK(reader.stats[0xA7] == 2);
		}
	}

	WHEN("file has one byte and number is read on 9 bits")
	{
		std::ofstream(path, std::ofstream::binary) << '\x01';
		Reader reader(path);
		reader.read(9);     // read on 9 bits
							// 0000'0001 -> 0000'0001|0
							//    number -> ^^^^^^^^^|* <- padding
		THEN("byte 0x01 occured once")
		{
			CHECK(reader.stats[0x01] == 1);
		}
		AND_WHEN("number is read while eof being set")
		{
			reader.read(7);  // eof is set so 0s will be output
			THEN("byte 0x00 occured once")
			{
				CHECK(reader.stats[0x00] == 1);
			}
		}
	}
	fs::remove(path);
}