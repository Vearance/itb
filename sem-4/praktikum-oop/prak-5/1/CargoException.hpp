#ifndef CARGO_EXCEPTION_HPP
#define CARGO_EXCEPTION_HPP

#include <exception>
#include <string>

class CargoException : public std::exception {
protected:
	int errorCode;
	std::string errorMessage;

public:
	CargoException(int code, const std::string& message) : errorCode(code), errorMessage(message) {}

	int getErrorCode() const {
		return this->errorCode;
	}

	const char* what() const noexcept override {
		return this->errorMessage.c_str();
	}
};

class CargoFullException : public CargoException {
public:
	CargoFullException() : CargoException(1, "Cargo is full!") {}
};

class CargoEmptyException : public CargoException {
public:
	CargoEmptyException() : CargoException(2, "Cargo is empty!") {}
};

class ItemNotFoundException : public CargoException {
public:
	ItemNotFoundException() : CargoException(3, "Item not found!") {}
};

class NotEnoughItemsException : public CargoException {
public:
	NotEnoughItemsException() : CargoException(4, "Not enough items in cargo!") {}
};

class TransferFailedException : public CargoException {
public:
	TransferFailedException() : CargoException(5, "Transfer failed due to destination capacity!") {}
};

#endif
