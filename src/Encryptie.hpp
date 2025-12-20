#pragma once

#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <string>
#include <print>

#include "aes.hpp"
#include "picosha2.h"

class Encryptie
{
public:

	using byte = uint8_t;
	using bytevec = std::vector<byte>;

	enum Status
	{
		FAIL,
		OK
	};

	struct Arguments
	{
		const std::string inputFile;
		const std::string outputFile;
		const std::string password;
	};

	Encryptie() = delete;
	~Encryptie() = delete;

	static bytevec generateIV()
	{
		bytevec iv(16);
		std::random_device rd;
		std::uniform_int_distribution<int> dist(0, 255);
		for (auto &b : iv)
			b = static_cast<byte>(dist(rd));
		return iv;
	}

	static bytevec hashPassword(const std::string &password)
	{
		bytevec hash(32);
		picosha2::hash256(password.begin(), password.end(), hash.begin(), hash.end());
		return hash;
	}

	static Status encryptFile(const Arguments &args)
	{
		bytevec key = hashPassword(args.password);
		bytevec iv = generateIV();

		std::ifstream file(args.inputFile, std::ios::binary);

		if (!file) {
			std::print(stderr, "Error: Could not open file {}!\n", args.inputFile);
			return FAIL;
		}

		bytevec buffer{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
		file.close();

		int paddingSize = 16 - (buffer.size() % 16);
		for (int i = 0; i < paddingSize; i++)
			buffer.push_back((byte)paddingSize);

		struct AES_ctx ctx;

		AES_init_ctx_iv(&ctx, key.data(), iv.data());

		AES_CBC_encrypt_buffer(&ctx, buffer.data(), buffer.size());

		std::ofstream outfile(args.outputFile, std::ios::binary);

		if (!outfile) {
			std::print(stderr, "Erorr: Could not open output file {}!\n", args.outputFile);
			return FAIL;
		}

		outfile.write(reinterpret_cast<const char *>(iv.data()), iv.size());
		outfile.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
		outfile.close();

		std::print("Encrypted {} successfully\n", args.inputFile);
		return OK;
	}

	static Status decryptFile(const Arguments &args)
	{
		std::ifstream file(args.inputFile, std::ios::binary);
		if (!file) {
			std::print(stderr, "Error: Could not open file {}!\n", args.inputFile);
			return FAIL;
		}

		bytevec data{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
		file.close();

		if (data.size() < 16) {
			std::print(stderr, "Error: File {} is too short!\n", args.inputFile);
			return FAIL;
		}

		bytevec iv(data.begin(), data.begin() + 16);
		bytevec ciphertext(data.begin() + 16, data.end());

		if (ciphertext.empty()) {
			std::print(stderr, "Error: File {} contains no encrypted data!\n", args.inputFile);
			return FAIL;
		}
		
		bytevec key = hashPassword(args.password);

		struct AES_ctx ctx;
		AES_init_ctx_iv(&ctx, key.data(), iv.data());

		AES_CBC_decrypt_buffer(&ctx, ciphertext.data(), ciphertext.size());

		byte paddingSize = ciphertext.back();

		if (paddingSize == 0 || paddingSize > 16) {
			std::print(stderr, "Error: Invalid padding size. Wrong password?\n");
			return FAIL;
		}

		if (paddingSize > ciphertext.size()) {
			std::print(stderr, "Error: Padding size exceeds data size!\n");
			return FAIL;
		}

		for (size_t i = ciphertext.size() - paddingSize; i < ciphertext.size(); i++) {
			if (ciphertext[i] != paddingSize) {
				std::print(stderr, "Error: Padding corrupt. Wrong password?\n");
				return FAIL;
			}
		}
		ciphertext.resize(ciphertext.size() - paddingSize);

		std::ofstream outfile(args.outputFile, std::ios::binary);

		if (!outfile) {
			std::print(stderr, "Error: Could not open output file {}!\n", args.outputFile);
			return FAIL;
		}

		outfile.write(reinterpret_cast<const char *>(ciphertext.data()), ciphertext.size());
		outfile.close();

		std::print("Decrypted {} successfully\n", args.inputFile);

		return OK;
	}

};
