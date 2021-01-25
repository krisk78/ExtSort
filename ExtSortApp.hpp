#pragma once

#include <ConsoleAppFW/consoleapp.hpp>
#include <utils/utils.hpp>

enum class FieldType
{
	alpha,
	numeric,
	date
};

class Field
{
public:
	FieldType type{FieldType::alpha};
	size_t position{ 0 };
	size_t length{ 0 };
};

class ExtSortApp : public ConsoleApp
{
public:
	ExtSortApp(bool window_mode) : ConsoleApp(window_mode) {};

	// final parameters storage
	std::string extension{ ".sor.txt" };
	char decSeparator{ '.' };
	std::string dateFormat{ "d.m.y" };
	std::string dateSeparator{ "." };
	int century{ 20 };
	bool fixedMode{ false };
	char fieldSeparator{ '\t' };
	std::vector<Field> keyFields{};
	bool reverse{ false };
	size_t begin{ 1 };
	bool double_precision{ false };
	bool ignore_overflow{ false };

	// argument names of the application
	const std::string FILE_ARG{ "file" };
	const std::string EXTENSION_ARG{ "extension" };
	const std::string DECIMAL_ARG{ "decimal" };
	const std::string DATEFMT_ARG{ "date" };
	const std::string FIELDSEP_ARG{ "field_sep" };
	const std::string FIELDPOS_ARG{ "field_pos" };
	const std::string FIXED_ARG{ "fixed" };
	const std::string REVERSE_ARG{ "reverse" };
	const std::string BEGIN_ARG{ "begin" };
	const std::string DOUBLE_ARG{ "double" };
	const std::string IGNORE_ARG{ "ignore" };

protected:
	virtual void SetUsage() override;											// Defines expected arguments and help.
	virtual std::string CheckArguments() override;								// Performs more accurate checks and initializations if necessary
	virtual void MainProcess(const std::filesystem::path& file) override;		// Launched by ByFile for each file matching argument 'file' values

private:
	strDateConverter dtConv;

	enum class Precision
	{
		simple_precision,
		double_precision
	};

	Precision precision{ Precision::simple_precision };

	enum class NumberPart
	{
		exponent,
		value
	};

	bool CheckDtFormat(const std::string& argvalue);
	bool AddFields(const std::string& argvalue, bool fixed = false);
	std::string makeSortableStr(const double dbl);
	std::string makeComplement(const std::string val, const NumberPart numPart);
};
