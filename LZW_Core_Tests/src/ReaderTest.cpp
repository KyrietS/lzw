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
				CHECK(reader.eof() == true);
			}
		}
	}

	GIVEN("Reader with eof flag set to true")
	{
		std::ofstream(path) << "a";
		Reader reader(path);
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
			}
			AND_THEN("eof flag is set")
			{
				CHECK(reader.eof() == true);
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
			}
			AND_THEN("eof flag is set")
			{
				CHECK(reader.eof() == true);
			}
		}
		WHEN("one 3-bit number is read")
		{
			uint64_t n = reader.read(3); // 101

			THEN("number is correct")
			{
				CHECK(n == 0b101);
			}
			AND_THEN("eof flag is not set")
			{
				CHECK(reader.eof() == false);
			}
		}
	}
	fs::remove(path);
}