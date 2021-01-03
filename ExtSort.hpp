#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <ConsoleAppFW/consoleapp.hpp>

class ExtSortApp : public ConsoleApp
{
public:
	ExtSortApp(bool window_mode) : ConsoleApp(window_mode) {};
protected:
	virtual void SetUsage() override;											// Defines expected arguments and help.
	virtual std::string CheckArguments() override;								// Performs more accurate checks and initializations if necessary
	virtual void MainProcess(const std::filesystem::path& file) override;		// Launched by ByFile for each file matching argument 'file' values
};

void ExtSortApp::SetUsage()
{
	us.description = "Sorts file(s) by given keys.";
	us.set_syntax("ExtSort.exe file [/o:extension] [/n:decimal_separator] [/d:date_format] ([/s:field_separator] /p:fields | /f:fixed) [/r] [/b:begin]");
	Unnamed_Arg file{ "file" };
	file.many = true;
	file.set_required(true);
	file.helpstring = "File(s) to sort.";
	us.add_Argument(file);
	Named_Arg o{ "extension" };
	o.switch_char = 'o';
	o.set_type(Argument_Type::string);
	o.set_default_value(".sor.txt");
	o.helpstring = "Extension of the output file(s), .sor.txt by default.";
	us.add_Argument(o);
	Named_Arg n{ "decimal" };
	n.switch_char = 'n';
	n.set_type(Argument_Type::string);
	n.set_default_value(".");
	n.helpstring = "Decimal separator, '.' by default.";
	us.add_Argument(n);
	Named_Arg d{ "date" };
	d.switch_char = 'd';
	d.set_type(Argument_Type::string);
	d.set_default_value("d.m.y");
	d.helpstring = "Date format, 'd.m.y' by default.";
	us.add_Argument(d);
	Named_Arg s{ "field_separator" };
	s.switch_char = 's';
	s.set_type(Argument_Type::string);
	s.set_default_value("\t");
	s.helpstring = "Field separator, tab by default.";
	us.add_Argument(s);
	Named_Arg p{ "field_positions" };
	p.switch_char = 'p';
	p.set_type(Argument_Type::string);
	p.set_required(true);
	p.helpstring = "Types and positions of the fields to sort, separated by a comma ','.";
	us.add_Argument(p);
	Named_Arg f{ "fixed" };
	f.switch_char = 'f';
	f.set_type(Argument_Type::string);
	f.set_required(true);
	f.helpstring = "Types, positions in chars and lengths of the fields to sort, separated by a comma ','.";
	us.add_Argument(f);
	Named_Arg r{ "reverse" };
	r.switch_char = 'r';
	r.set_type(Argument_Type::simple);
	r.helpstring = "Perform a sort in reverse order.";
	us.add_Argument(r);
	Named_Arg b{ "begin" };
	b.switch_char = 'b';
	b.set_type(Argument_Type::string);
	b.helpstring = "Number of the line from which the sort must be performed.";
	us.add_Argument(b);
	us.add_requirement(s.name(), p.name());
	us.add_conflict(p.name(), f.name());
	us.usage =	"Date field position must be preceded by the 'D' char and numeric field position by the 'N' char.\n"
				"When using fixed positions the position and the length are separated by the 'L' char, ie. 3L8.\n\n"
				"Examples:\n\n"
				"ExtSort foo.txt /p:2,D5 /b:8\n"
				"    Creates the file foo.sor.txt ordered from the 8th line based on 2nd and 5th fields.\n"
				"    The 2nd field is sorted by alphanumerical order and the 5th field is ordered by time order.\n"
				"    The fields are supposed to be separated by a tab.\n\n"
				"ExtSort foo.txt /f:35L5,N3L8 /r\n"
				"    Creates the file foo.sor.txt ordered from the 1st line based on 2 fields.\n"
				"    The 1st field starts at position 35 with length 5 and the 2nd starts at position 3 with length 8.\n"
				"	 The 2nd field is sorted by numerical values.\n";
}

std::string ExtSortApp::CheckArguments()
{

}

void ExtSortApp::MainProcess(const std::filesystem::path& file)
{

}