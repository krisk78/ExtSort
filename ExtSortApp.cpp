//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

#include "ExtSortApp.hpp"
#include <utils/utils.hpp>

void ExtSortApp::SetUsage()
{
	us.description = "Sorts file(s) by given keys.";
	us.set_syntax("ExtSort.exe " + FILE_ARG + " [/o:" + EXTENSION_ARG + "] [/n:" + DECIMAL_ARG + "] [/d:" + DATEFMT_ARG + "]\n"
		"                ([/s:" + FIELDSEP_ARG + "] /p:" + FIELDPOS_ARG + " | /f:" + FIXED_ARG + ") [/r] [/b:" + BEGIN_ARG + "]\n"
		"                [/double] [/i]");
	
	Unnamed_Arg file{ FILE_ARG };
	file.many = true;
	file.set_required(true);
	file.helpstring = "File(s) to sort.";
	us.add_Argument(file);
	
	Named_Arg o{ EXTENSION_ARG };
	o.switch_char = 'o';
	o.set_type(Argument_Type::string);
	o.set_default_value(extension);
	o.helpstring = "Extension of the output file(s).";
	us.add_Argument(o);
	
	Named_Arg n{ DECIMAL_ARG };
	n.switch_char = 'n';
	n.set_type(Argument_Type::string);
	n.set_default_value("" + decSeparator);
	n.helpstring = "Decimal separator.";
	us.add_Argument(n);
	
	Named_Arg d{ DATEFMT_ARG };
	d.switch_char = 'd';
	d.set_type(Argument_Type::string);
	d.set_default_value(dateFormat);
	d.helpstring = "Date format.";
	us.add_Argument(d);
	
	Named_Arg s{ FIELDSEP_ARG };
	s.switch_char = 's';
	s.set_type(Argument_Type::string);
	s.set_default_value("" + fieldSeparator);
	s.helpstring = "Field separator.";
	us.add_Argument(s);
	
	Named_Arg p{ FIELDPOS_ARG };
	p.switch_char = 'p';
	p.set_type(Argument_Type::string);
	p.set_required(true);
	p.helpstring = "Types and positions of the fields to sort, separated\n"
		"by a comma ','.";
	us.add_Argument(p);
	
	Named_Arg f{ FIXED_ARG };
	f.switch_char = 'f';
	f.set_type(Argument_Type::string);
	f.set_required(true);
	f.helpstring = "Types, positions in chars and lengths of the fields\n"
		"to sort, separated by a comma ','.";
	us.add_Argument(f);
	
	Named_Arg r{ REVERSE_ARG };
	r.switch_char = 'r';
	r.set_type(Argument_Type::simple);
	r.helpstring = "Perform a sort in reverse order.";
	us.add_Argument(r);
	
	Named_Arg b{ BEGIN_ARG };
	b.switch_char = 'b';
	b.set_type(Argument_Type::string);
	b.helpstring = "Number of the line from which the records must be\n"
		"sorted.";
	us.add_Argument(b);
	
	Named_Arg dbl{ DOUBLE_ARG };
	dbl.set_type(Argument_Type::simple);
	dbl.helpstring = "Use double precision to evaluate numeric values.";
	us.add_Argument(dbl);
	
	Named_Arg i{ IGNORE_ARG };
	i.switch_char = 'i';
	i.set_type(Argument_Type::simple);
	i.helpstring = "Ignore overflow errors.";
	us.add_Argument(i);
	
	us.add_requirement(s.name(), p.name());
	us.add_conflict(p.name(), f.name());
	
	us.usage = "A date field position must be preceded by the 'D' char and a numeric field\n"
		"position by the 'N' char.\n\n"
		"When using fixed positions the position and the length are separated by the 'L'\n"
		"char, ie. 3L8.\n\n"
		"Using the option /p, if the length of a string field to sort is variable, then\n"
		"the maximum length of the field must be given with the same syntax used to set\n"
		"the length of the fixed position argument, using 'L' char.\n"
		"This form is not allowed for date and numeric fields.\n\n"
		"If the option /i is used, fields that length is greater than the expected length\n"
		"are truncated on the right in the key used to perform the sort. In this case the\n"
		"sort is not totally ensured.\n"
		"For numeric fields, an error still occurs if the value exceeds the double float\n"
		"capacity. If not, exceeding right significant digits are ignored while using\n"
		"simple precision with double values.\n"
		"An error still occurs if the length of the exponent is greater that 2 digits\n"
		"without using double float precision.\n\n"
		"Examples:\n\n"
		"ExtSort foo.txt /p:2,D5 /b:8\n"
		"    Creates the file foo.sor.txt ordered from the 8th line based on 2nd and 5th\n"
		"    fields. The 2nd field is sorted by alphanumerical order and the 5th field\n"
		"    is ordered by time order. The fields are supposed to be separated by a tab.\n\n"
		"ExtSort foo.txt /f:35L5,N3L8 /r\n"
		"    Creates the file foo.sor.txt ordered from the 1st line based on 2 fields.\n"
		"    The 1st field starts at position 35 with length 5 and the 2nd starts at\n"
		"    position 3 with length 8. The 2nd field is sorted by numerical values.\n";
}

std::string ExtSortApp::CheckArguments()
{
	const std::string HELP_MESSAGE = " not valid. See /? for help.";

	auto ext = us.get_Argument(EXTENSION_ARG);
	if (!ext->value.empty() && !ext->value.front().empty())
	{
		extension = ext->value.front();
		if (extension[0] != '.')
			extension.insert(0, ".");
	}

	auto decs = us.get_Argument(DECIMAL_ARG);
	if (!decs->value.empty() && !decs->value.front().empty())
		decSeparator = decs->value.front()[0];

	auto dfmt = dynamic_cast<Named_Arg*>(us.get_Argument(DATEFMT_ARG));
	if (!dfmt->value.empty())
		if (auto dform = dfmt->value.front(); dform != dfmt->default_value())		// a user-defined date format is set
			if (!CheckDtFormat(dform))
				return "Date format '" + dform + "' is" + HELP_MESSAGE;
	auto t = std::time(0);
	std::tm now;
	localtime_s(&now, &t);
	century = now.tm_year / 100 + 19;

	// initialize the date conversion fastener
	dtConv.setFormats(dateFormat, "yyyymmdd");
	dtConv.century = century;
	assert((dtConv.isValid(strDateConverter::BOTH) == strDateConverter::BOTH) && "Fatal issue has occurred in date format validation.");

	auto fsep = us.get_Argument(FIELDSEP_ARG);
	if (!fsep->value.empty() && !fsep->value.front().empty())
		fieldSeparator = fsep->value.front()[0];

	auto fieldp = us.get_Argument(FIELDPOS_ARG);
	if (!fieldp->value.empty())				// field positions with delimiter are used instead of fixed positions with length
	{
		if (!AddFields(fieldp->value.front()))
			return "Field positions '" + fieldp->value.front() + "' are" + HELP_MESSAGE;
	}

	auto fixedp = us.get_Argument(FIXED_ARG);
	if (!fixedp->value.empty())				// fixed positions with length are used instead field positions with delimiter
	{
		fixedMode = true;
		if (!AddFields(fixedp->value.front(), true))
			return "Fixed positions '" + fixedp->value.front() + "' are" + HELP_MESSAGE;
	}

	auto rev = us.get_Argument(REVERSE_ARG);
	if (!rev->value.empty() && rev->value.front() == "true")
		reverse = true;

	auto beg = us.get_Argument(BEGIN_ARG);
	if (!beg->value.empty())
	{
		auto begv = beg->value.front();
		if (std::find_if(begv.begin(), begv.end(), [](unsigned char c) {return !std::isdigit(c); }) == begv.end())
			begin = std::stoi(begv);
		else
			return "Begin value '" + begv + "' is" + HELP_MESSAGE;
	}

	auto dbl = us.get_Argument(DOUBLE_ARG);
	if (!dbl->value.empty() && dbl->value.front() == "true")
	{
		double_precision = true;
		precision = Precision::double_precision;
	}

	auto ign = us.get_Argument(IGNORE_ARG);
	if (!ign->value.empty() && ign->value.front() == "true")
		ignore_overflow = true;

	return "";				// all is okay
}

void ExtSortApp::MainProcess(const std::filesystem::path& file)
{
	std::string NUM_CHARS = "-0123456789" + decSeparator;
	static const std::string CMD_LINE{ "sort " };
	static const unsigned long long DEFAULT_INCREMENT = 1000;
	static const unsigned long long AVER_ROW_LEN = 120;

	// initialize input file
	auto fsize = std::filesystem::file_size(file);
	auto EOL_type = file_EOL(file);
	auto EOL_len = EOL_length(EOL_type);
	char EOL_delim = '\n';
	if (EOL_type == EOL::Mac)
		EOL_delim = '\r';
	std::ifstream infile(file, std::ios::binary);
	std::uintmax_t lineCnt{ 0 };
	
	// initialize output file
	auto outpath = getOutPath(file);
	if (std::filesystem::exists(outpath))
		std::filesystem::remove(outpath);
	std::ofstream outfile(outpath, std::ios_base::out | std::ios::binary);
	std::uintmax_t outCnt{ 0 };
	
	// initialize temp output file
	std::string tmpname{ file.generic_string() };
	tmpname.append(".tmp");
	std::filesystem::path tmppath(tmpname);
	if (std::filesystem::exists(tmppath))
		std::filesystem::remove(tmppath);
	std::ofstream tmpfile(tmppath, std::ios_base::out);
	std::uintmax_t tmpCnt{ 0 };

	// initialize temp input file
	std::string sortname{ tmpname };
	sortname.append(".sorted");
	std::filesystem::path sortpath(sortname);

	// copy header lines
	std::string buf;
	while (lineCnt < (begin - 1) && !infile.eof())
	{
		std::getline(infile, buf, EOL_delim);
		if (buf.length() != 0 && EOL_type == EOL::Windows)
			if (buf.back() == '\r')
				buf.pop_back();
		outfile << buf << EOL_str(EOL_type);
		lineCnt++;
		outCnt++;
	}

	// index creation
	long long currPos = infile.tellg();
	unsigned long long increment;
	if ((increment = ((fsize / AVER_ROW_LEN / 100) / DEFAULT_INCREMENT) * DEFAULT_INCREMENT) < DEFAULT_INCREMENT)
		increment = DEFAULT_INCREMENT;
	while (infile)
	{
		std::getline(infile, buf, EOL_delim);
		lineCnt++;
		if (buf.length() != 0)
		{
			if (EOL_type == EOL::Windows)
				if (buf.back() == '\r')
					buf.pop_back();
			std::string key{};
			std::vector<std::string> fields;
			auto parsed = fields.size();
			for (size_t fcnt = 0; fcnt < keyFields.size(); fcnt++)
			{
				std::string field{};
				if (fixedMode)			// fields are defined by position in chars and length
				{
					if (buf.length() >= keyFields[fcnt].position)
						field = buf.substr(keyFields[fcnt].position - 1, keyFields[fcnt].length);
				}
				else					// fields are defined by field number with delimiter
				{
					if (parsed == 0)
					{
						fields = split(buf, fieldSeparator);
						parsed = fields.size();
					}
					if (parsed >= keyFields[fcnt].position)
						field = fields[keyFields[fcnt].position - 1];
				}
				switch (keyFields[fcnt].type)
				{
				case FieldType::alpha:
					field.resize(keyFields[fcnt].length, ' ');
					key += field;
					break;
				case FieldType::date:
					if (field.length() == 0)
					{
						field.resize(8, ' ');
						key += field;
					}
					else
						key += dtConv.convStrDate(field);
					break;
				case FieldType::numeric:
					trim(field);
					if (field.back() == '-')
					{
						field.pop_back();
						field.insert(field.begin(), '-');
					}
					// remove all that is not the sign, a digit or the decimal point
					auto pos = field.find_first_not_of(NUM_CHARS);
					while (pos != std::string::npos)
					{
						field.erase(pos);
						pos = field.find_first_not_of(NUM_CHARS, pos);
					}
					double dblvalue;
					try {
						dblvalue = stod(field);
						key += makeSortableStr(dblvalue); }
					catch (const std::overflow_error&) {
						throw; }
					catch (const std::out_of_range&) {
						throw; }
					catch (...) {		// not a number
						if (double_precision)
							field.resize(22, ' ');
						else
							field.resize(12, ' ');
						key += field;
					}
				}
			}
			tmpfile << key << '\t' << currPos << "\n";
			tmpCnt++;
		}
		if ((currPos = infile.tellg()) == -1)
			currPos = fsize;
		if (lineCnt % increment == 0 || !infile)
			std::cout << "\rReading " << file.filename() << " : " << lineCnt << " lines (" << currPos * 100 / fsize << "%)";
	}
	std::cout << std::endl;
	tmpfile.close();
	// sort tmp file
	std::cout << "Sort indexes..." << std::endl;
	std::string command{ CMD_LINE + tmpname + " /O " + sortname };
	auto ret = std::system(command.c_str());
	std::filesystem::remove(tmppath);
	if (ret != 0)
		throw std::system_error(std::make_error_code(std::errc::interrupted), "Error while performing sort.");
	// open sorted tmp file
	fsize = std::filesystem::file_size(sortpath);
	std::ifstream sortfile(sortpath);
	std::uintmax_t sortCnt{ 0 };
	// read sorted indexes and write matching lines of input file to output file
	infile.clear();
	if ((increment = ((fsize / AVER_ROW_LEN / 100) / DEFAULT_INCREMENT) * DEFAULT_INCREMENT) < DEFAULT_INCREMENT)
		increment = DEFAULT_INCREMENT;
	while (sortfile)
	{
		std::getline(sortfile, buf);
		sortCnt++;
		if (buf.length() != 0)
		{
			auto pos = buf.find('\t');
			if (pos != std::string::npos)
			{
				long long index = stoll(buf.substr(pos + 1, buf.length() - pos));
				infile.seekg(index);
				std::getline(infile, buf, EOL_delim);
				if (buf.length() != 0 && EOL_type == EOL::Windows)
					if (buf.back() == '\r')
						buf.pop_back();
				outfile << buf << EOL_str(EOL_type);
				outCnt++;
			}
		}
		if ((currPos = sortfile.tellg()) == -1)
			currPos = fsize;
		if (outCnt % increment == 0 || !sortfile)
			std::cout << "\rWriting " << outpath.filename() << " : " << outCnt << " lines (" << currPos * 100 / fsize << "%)";
	}
	std::cout << "\n" << std::endl;
	sortfile.close();
	std::filesystem::remove(sortpath);
	infile.close();
	outfile.close();
}

bool ExtSortApp::CheckDtFormat(const std::string& argvalue)
{
	dateFormat = argvalue;
	dateSeparator = "";
	dateFormat = to_lower(dateFormat);
	auto day = dateFormat.find('j');
	while (day != std::string::npos)		// replace french 'j' for days by 'd'
	{
		dateFormat[day] = 'd';
		day = dateFormat.find('j');
	}
	auto year = dateFormat.find('a');
	while (year != std::string::npos)		// replace french 'a' for years by 'y'
	{
		dateFormat[year] = 'y';
		year = dateFormat.find('a');
	}
	if (auto dsep = dateFormat.find_first_not_of("dmy"); dsep != std::string::npos)
		dateSeparator = dateFormat[dsep];
	auto fmtlen = dateFormat.size();
	if (auto other = dateFormat.find_first_not_of("dmy" + dateSeparator); other != std::string::npos)
		fmtlen = 0;
	if (!dateSeparator.empty())
	{
		fmtlen -= 2;
		if (std::count(dateFormat.begin(), dateFormat.end(), dateSeparator[0]) != 2)
			fmtlen = 0;
	}
	if (fmtlen != 3)
		fmtlen = 0;
	if (std::count(dateFormat.begin(), dateFormat.end(), 'd') != 1 || std::count(dateFormat.begin(), dateFormat.end(), 'm') != 1
		|| std::count(dateFormat.begin(), dateFormat.end(), 'y') != 1)
		fmtlen = 0;
	if (fmtlen == 0)
		return false;
	return true;
}

bool ExtSortApp::AddFields(const std::string & argvalue, bool fixed)
{
	std::istringstream fields{ to_lower(argvalue) };
	std::string field;
	while (std::getline(fields, field, ','))
	{
		Field key;
		bool parsed{ false };
		if (!field.empty())
		{
			if (field[0] == 'd')
				key.type = FieldType::date;
			if (field[0] == 'n')
				key.type = FieldType::numeric;
			if (field[0] == 'd' || field[0] == 'n')
				field.erase(0, 1);
		}
		if (!field.empty())
			if (auto lsep = field.find_first_of('l'); lsep != std::string::npos)
			{
				auto len = field.substr(lsep + 1, field.size() - lsep);
				if (std::find_if(len.begin(), len.end(), [](unsigned char c) {return !std::isdigit(c); }) == len.end())
				{
					key.length = std::stoi(len);
					field = field.substr(0, lsep);
				}
			}
		if (!field.empty())
			if (std::find_if(field.begin(), field.end(), [](unsigned char c) {return !std::isdigit(c); }) == field.end())
			{
				key.position = std::stoi(field);
				keyFields.push_back(key);
				parsed = true;
			}
		if (!parsed || fixed && key.length == 0 || !fixed && key.length != 0 && key.type != FieldType::alpha)
			return false;
	}
	return true;
}

std::string ExtSortApp::makeComplement(const std::string val, const NumberPart numPart)
{
	static const std::string COMPL_EXP_SIMPLE{ "99" };
	static const std::string COMPL_EXP_DOUBLE{ "999" };
	static const std::string COMPL_VAL_SIMPLE{ "99999999" };
	static const std::string COMPL_VAL_DOUBLE{ "99999999999999999" };

	const std::string* complement;
	if (numPart == NumberPart::exponent)
	{
		complement = &COMPL_EXP_SIMPLE;
		if (precision == Precision::double_precision)
			complement = &COMPL_EXP_DOUBLE;
	}
	else
	{
		complement = &COMPL_VAL_SIMPLE;
		if (precision == Precision::double_precision)
			complement = &COMPL_VAL_DOUBLE;
	}
	std::string result = *complement;
	size_t lencompl = result.length();
	size_t lenval = val.length();
	size_t offset = lencompl - lenval;
	if (offset < 0)
	{
		if (!ignore_overflow || numPart == NumberPart::exponent)
			throw std::overflow_error("Value exceeds the given precision.");
		offset = 0;
		lenval = lencompl;
	}
	for (size_t i = 0; i < lenval; i++)
		result[offset + i] -= val[i];
	return result;
}

std::string ExtSortApp::makeSortableStr(const double dbl)
{
	// convert value to a sortable string
	// sign 0(-)/1(+), exponent sign 0/1, fixed length exponent and value digits without decimal dot
	// exponent sign must be inverted for negative values

	static const std::string BUF_VAL_SIMPLE{ "00000000" };
	static const std::string BUF_VAL_DOUBLE{ "00000000000000000" };

	const std::string* BUF_VAL = (precision == Precision::simple_precision) ? &BUF_VAL_SIMPLE : &BUF_VAL_DOUBLE;
	std::string result{};
	std::string scfmt = get_message("%e", dbl);
	auto pos = scfmt.find('e');
	auto sclen = scfmt.length();
	assert((pos != std::string::npos && sclen >= pos + 4 ) && "Fatal error while formatting a double value to a string.");
	auto exp = scfmt.substr(pos + 2, sclen - pos - 1);
	auto value = scfmt.substr(0, pos - 1);
	bool complexp{ false };
	bool complval{ false };
	if (dbl < 0)
	{
		value.erase(0);		// no sign
		value.erase(1);		// no decimal dot
		result.push_back('0');
		complexp = (scfmt[pos + 1] != '-');
		complval = true;
	}
	else
	{
		value.erase(1);		// no decimal dot
		result.push_back('1');
		complexp = (scfmt[pos + 1] == '-');
	}
	if (complexp)
	{
		result.push_back('0');
		try {
			result += makeComplement(exp, NumberPart::exponent); }
		catch (...) {
			throw std::overflow_error("Exponent exceeds the given precision."); }
	}
	else
	{
		result.push_back('1');
		if (!ignore_overflow && precision == Precision::simple_precision && exp.length() > 2)
			throw std::overflow_error("Exponent exceeds the given precision.");
		if (precision == Precision::double_precision && exp.length() < 3)
			result.push_back('0');
		result += exp;
	}
	if (complval)
	{
		try {
			result += makeComplement(value, NumberPart::value); }
		catch (...) {
			throw; }
	}
	else
	{
		auto vallen = value.length();
		auto buflen = (*BUF_VAL).length();
		if (vallen > buflen)
		{
			if (!ignore_overflow)
				throw std::overflow_error("Value exceeds the given precision.");
			value = value.substr(0, buflen);
		}
		if (vallen < buflen)
			result += (*BUF_VAL).substr(0, buflen - vallen);
		result += value;
	}
	return result;
}
