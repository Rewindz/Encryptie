#include <iostream>
#include <print>
#include <span>
#include <string>
#include <filesystem>

#include "Encryptie.hpp"
#include "TerminalPassword.hpp"

enum class Mode
{
	Error,
	Encrypt,
	Decrypt
};

void printUsage(const char *prog) 
{
	std::print("Usage: {} -i <input> -o <output> [--encrypt|--decrypt]\n", prog);
}

void printHelp()
{
	std::print("Encryptie -- A Simple File Encryption/Decryption Program\n");
	std::print("\t-i | --input <input file>            Selects a file for encryption\n");
	std::print("\t-o | --output <output file>          Selects the output file\n");
	std::print("\t-e | --encrypt                       The input will be encrypted\n");
	std::print("\t-d | --decrypt                       The input will be decrypted\n");
	std::print("\t-p | --password                      Sets the password (be aware this will be in shell history)\n");
	std::print("\t-r | --remove | --delete             Delete the input file after encryption (encryption mode only)\n");
	std::print("\t-h | --help                          Prints this message\n");
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printUsage(argv[0]);
		return 1;
	}

	std::span cargs{ argv + 1, static_cast<size_t>(argc - 1) };
	
	std::string inputFile, outputFile, password;
	Mode encryptMode = Mode::Error;

	bool removeFlag = false;

	for (size_t i = 0; i < cargs.size(); i++) {
		std::string arg = cargs[i];
		if (arg == "-i" || arg == "--input") {
			if (i + 1 < cargs.size())
				inputFile = cargs[++i];
			else {
				std::print(stderr, "Error: {} requires a filename!\n", arg);
				return 1;
			}
		}
		else if (arg == "-o" || arg == "--output") {
			if (i + 1 < cargs.size())
				outputFile = cargs[++i];
			else {
				std::print(stderr, "Error: {} requires a filename!\n", arg);
				return 1;
			}
		}
		else if (arg == "-p" || arg == "--password") {
			if (i + 1 < cargs.size())
				password = cargs[++i];
			else {
				std::print(stderr, "Error: {} requires a password!\n", arg);
				return 1;
			}
		}
		else if (arg == "--decrypt" || arg == "-d")
			encryptMode = Mode::Decrypt;
		else if (arg == "--encrypt" || arg == "-e")
			encryptMode = Mode::Encrypt;
		else if (arg == "-r" || arg == "--remove" || arg == "--delete")
			removeFlag = true;
		else if (arg == "-h" || arg == "--help"){
			printHelp();
			return 0;
		}
		else {
			std::print(stderr, "Unknown argument: {}\n", arg);
			return 1;
		}
	}

	if(removeFlag && encryptMode == Mode::Encrypt){
		std::print("The remove input file is set. This will delete the input file after encryption\n");
		std::print("Are you sure you want to continue? Y/n\n");
		std::string confirm;
		std::cin >> confirm;
		if(confirm[0] != 'Y' && confirm[0] != 'y'){
			std::print("Cancelling.");
			return 0;
		}
	}

	if (password.empty()) {
		bool first = true;
		std::string confirm;
		std::print("Please enter password:");
		do {
			if (!first) std::print("Passwords are not the same! Re-enter password:");
			SetEcho(false);
			std::cin >> password;
			SetEcho(true);
			assert(password.back() != '\n');
			std::print("Confirm password:");
			SetEcho(false);
			std::cin >> confirm;
			SetEcho(true);
			first = false;
		} while (password != confirm);
		std::fill(confirm.begin(), confirm.end(), 0);
	}

	if (inputFile.empty() || outputFile.empty() || password.empty() || encryptMode == Mode::Error) {
		std::print(stderr, "Error: Missing required arguments.\n");
		printUsage(argv[0]);
		return 1;
	}

	Encryptie::Arguments encArgs{
		.inputFile = inputFile, 
		.outputFile = outputFile, 
		.password = password
	};

	Encryptie::Status res = Encryptie::FAIL;
	switch (encryptMode)
	{
		case Mode::Encrypt:
			res = Encryptie::encryptFile(encArgs);
			if(res == Encryptie::OK && removeFlag && !inputFile.empty()){
				std::filesystem::remove(inputFile);
			}
			break;
		case Mode::Decrypt:
			res = Encryptie::decryptFile(encArgs);
			break;
		default:
			std::print(stderr, "Error: State is unknown!\n");
			break;
	}

	std::fill(password.begin(), password.end(), 0);


	if (res == Encryptie::FAIL)
		std::print("Operation failed!\n");


	std::print("\n");
	return res == Encryptie::FAIL;
}