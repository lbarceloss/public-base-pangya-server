#include <filesystem>
#include <iostream>
#include <string>
#include <exception>

int main() {

	try {

		std::string dir = "Loge";

		auto s = std::filesystem::status(dir);

		if (!std::filesystem::exists(s))
			std::cout << "Não existe" << std::endl;

	} catch(std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}
