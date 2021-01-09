//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>

#include "ExtSortApp.hpp"
#include <utils/utils.hpp>

void ExtSortApp::SetUsage()
{
	us.description = "Sorts file(s) by given keys.";
	us.set_syntax("ExtSort.exe " + FILE_ARG + " [/o:" + EXTENSION_ARG + "] [/n:" + DECIMAL_ARG + "] [/d:" + DATEFMT_ARG + "]\n"
		"                ([/s:" + FIELDSEP_ARG + "] /p:" + FIELDPOS_ARG + " | /f:" + FIXED_ARG + ") [/r] [/b:" + BEGIN_ARG + "]");
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
	us.add_requirement(s.name(), p.name());
	us.add_conflict(p.name(), f.name());
	us.usage = "A date field position must be preceded by the 'D' char and a numeric field\n"
		"position by the 'N' char.\n"
		"When using fixed positions the position and the length are separated by the 'L'\n"
		"char, ie. 3L8.\n\n"
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

	auto fsep = us.get_Argument(FIELDSEP_ARG);
	if (!fsep->value.empty() && !fsep->value.front().empty())
		fieldSeparator = fsep->value.front()[0];

	auto fieldp = us.get_Argument(FIELDPOS_ARG);
	if (!fieldp->value.empty())				// field positions with delimiter are used instead of fixed positions with length
		if (!AddFields(fieldp->value.front()))
			return "Field positions '" + fieldp->value.front() + "' are" + HELP_MESSAGE;

	auto fixedp = us.get_Argument(FIXED_ARG);
	if (!fixedp->value.empty())				// fixed positions with length are used instead field positions with delimiter
		if (!AddFields(fixedp->value.front(), true))
			return "Fixed positions '" + fixedp->value.front() + "' are" + HELP_MESSAGE;

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

	return "";				// all is okay
}

void ExtSortApp::MainProcess(const std::filesystem::path& file)
{
	const size_t COMPLEMENT_VALUE = 99999999;
	const size_t BUF_LENGTH = 4096;

	// initialize input file
	auto fsize = std::filesystem::file_size(file);
	auto EOL_len = EOL_length(file_EOL(file));
	std::ifstream infile(file);
	std::uintmax_t lineCnt{ 0 };
	
	// initialize output file
	auto outpath = getOutPath(file);
	if (std::filesystem::exists(outpath))
		std::filesystem::remove(outpath);
	std::ofstream outfile(outpath, std::ios_base::out);
	std::uintmax_t outCnt{ 0 };
	
	// initialize temp file
	std::string tmpname{ file.generic_string() };
	tmpname.append(".tmp");
	std::filesystem::path tmppath(tmpname);
	if (std::filesystem::exists(tmppath))
		std::filesystem::remove(tmppath);
	std::ofstream tmpfile(tmppath, std::ios_base::out);
	std::uintmax_t tmpCnt{ 0 };

	// copy header lines
	std::string buf;
	while (lineCnt < (begin - 1) && !infile.eof())
	{
		std::getline(infile, buf);
		outfile << buf << std::endl;
		lineCnt++;
		outCnt++;
	}

	// création des index
	while (!infile.eof())
	{
		std::getline(infile, buf);
	}
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
		if (!field.empty() && fixed)
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
		if (!parsed || fixed && key.length == 0)
			return false;
	}
	return true;
}

std::filesystem::path ExtSortApp::getOutPath(const std::filesystem::path& inpath)
{
	std::string outname{ inpath.generic_string() };
	auto extarg = us.get_Argument("extension");
	if (extarg == NULL || !Arguments_Checked())
		return std::filesystem::path(outname);
	auto ext = outname.find_last_of('.');
	if (ext != std::string::npos)
		outname.erase(ext, outname.size() - ext);
	outname.append(extarg->value.front());
	return std::filesystem::path(outname);
}