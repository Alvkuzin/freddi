#include "output.hpp"

#include <sstream>

#include "unit_transformation.hpp"

BasicFreddiFileOutput::BasicFreddiFileOutput(const std::shared_ptr<FreddiEvolution>& freddi, const boost::program_options::variables_map& vm,
											 const std::vector<FileOutputShortField>&& short_fields, const std::vector<FileOutputLongField>&& long_fields):
		freddi(freddi),
		output(freddi->args().general->dir + "/" + freddi->args().general->prefix + ".dat"),
		short_fields(short_fields),
		long_fields(long_fields),
		fulldata_header(initializeFulldataHeader()) {
	output << "#" << short_fields.at(0).name;
	for (size_t i = 1; i < short_fields.size(); ++i) {
		output << "\t" << short_fields[i].name;
	}
	output << "\n";

	output << "#" << short_fields.at(0).unit;
	for (size_t i = 1; i < short_fields.size(); ++i) {
		output << "\t" << short_fields[i].unit;
	}
	output << "\n";

	for (const auto &it : vm) {
		auto &value = it.second.value();
		if (auto v = boost::any_cast<uint32_t>(&value)) {
			output << "# "
				   << it.first.c_str()
				   << "="
				   << *v
				   << "\n";
		} else if (auto v = boost::any_cast<std::string>(&value)) {
			output << "# "
				   << it.first.c_str()
				   << "="
				   << *v
				   << "\n";
		} else if (auto v = boost::any_cast<double>(&value)) {
			output << "# "
				   << it.first.c_str()
				   << "="
				   << *v
				   << "\n";
		} else if (auto v = boost::any_cast<unsigned int>(&value)) {
			output << "# "
				   << it.first.c_str()
				   << "="
				   << *v
				   << "\n";
		} else if (auto v = boost::any_cast<std::vector<double> >(&value)) {
			for (int i = 0; i < v->size(); ++i) {
				output << "# "
					   << it.first.c_str()
					   << "="
					   << v->at(i)
					   << "  # "
					   << i
					   << "\n";
			}
		} else if (auto v = boost::any_cast<std::vector<std::string> >(&value)) {
			for (int i = 0; i < v->size(); ++i) {
				output << "# "
					   << it.first.c_str()
					   << "="
					   << v->at(i)
					   << "  # "
					   << i
					   << "\n";
			}
		} else {
//			output << "error\n";
			throw boost::program_options::invalid_option_value(it.first.c_str());
		}
	}
	if (vm.count("rout") == 0) {
		output << "# --rout hadn't been specified, tidal radius " << freddi->args().basic->rout / solar_radius << " Rsun was used"
			   << std::endl;
	}
	output << std::flush;
}


std::string BasicFreddiFileOutput::initializeFulldataHeader() const {
	std::string s;

	s += "#" + long_fields.at(0).name;
	for (size_t i = 1; i < long_fields.size(); ++i) {
		s += "\t" + long_fields[i].name;
	}
	s += "\n";

	s += "#" + long_fields.at(0).unit;
	for (size_t i = 1; i < long_fields.size(); ++i) {
		 s += "\t" + long_fields[i].unit;
	}
	s += "\n";

	return s;
}

void BasicFreddiFileOutput::shortDump() {
	output << short_fields[0].func();
	for (size_t i = 1; i < short_fields.size(); ++i) {
		output << "\t" << short_fields[i].func();
	}
	output << std::endl;
}

void BasicFreddiFileOutput::longDump() {
	std::ostringstream filename;
	auto i_t = static_cast<int>(std::round(freddi->t() / freddi->args().calc->tau));
	filename << freddi->args().general->dir << "/" << freddi->args().general->prefix << "_" << i_t << ".dat";
	FstreamWithPath full_output(filename.str());

	full_output << fulldata_header
			<< "# Time = " << sToDay(freddi->t())
			<< " Mdot_in = " << freddi->Mdot_in()
			<< std::endl;

	for ( int i = freddi->first(); i <= freddi->last(); ++i ){
		full_output << long_fields.at(0).func()[i];
		for (size_t j = 1; j < long_fields.size(); ++j) {
			full_output << "\t" << long_fields[j].func()[i];
		}
		full_output << "\n";
	}
	full_output << std::flush;
}

void BasicFreddiFileOutput::dump() {
	shortDump();

	if (freddi->args().general->fulldata) {
		longDump();
	}
}


std::vector<FileOutputShortField> FreddiFileOutput::initializeShortFields(const std::shared_ptr<FreddiEvolution>& freddi) {
	std::vector<FileOutputShortField> fields {
			{"t", "days", [freddi]() {return sToDay(freddi->t());}},
			{"Mdot", "g/s",  std::bind(&FreddiEvolution::Mdot_in, freddi)},
			{"Mdisk", "g", std::bind(&FreddiEvolution::Mdisk, freddi)},
			{"Rhot", "Rsun", [freddi]() {return cmToSun(freddi->R()[freddi->last()]);}},
			{"Cirrout", "float", [freddi]() {return freddi->Cirr()[freddi->last()];}},
			{"H2R", "float", [freddi]() {return freddi->Height()[freddi->last()] / freddi->R()[freddi->last()];}},
			{"Teffout", "K", [freddi]() {return freddi->Tph()[freddi->last()];}},
			{"Tirrout", "K", [freddi]() {return freddi->Tirr()[freddi->last()];}},
			{"Qiir2Qvisout", "float", [freddi]() {return pow(freddi->Tirr()[freddi->last()] / freddi->Tph_vis()[freddi->last()], 4.);}},
			{"Lx", "erg/s", std::bind(&FreddiEvolution::Lx, freddi)},
			{"mU", "mag", std::bind(&FreddiEvolution::mU, freddi)},
			{"mB", "mag", std::bind(&FreddiEvolution::mB, freddi)},
			{"mV", "mag", std::bind(&FreddiEvolution::mV, freddi)},
			{"mR", "mag", std::bind(&FreddiEvolution::mR, freddi)},
			{"mI", "mag", std::bind(&FreddiEvolution::mI, freddi)},
			{"mJ", "mag", std::bind(&FreddiEvolution::mJ, freddi)},
	};
	const auto& lambdas = freddi->args().flux->lambdas;
	for (size_t i = 0; i < lambdas.size(); ++i) {
		const double lambda = lambdas[i];
		fields.push_back(FileOutputShortField{
				std::string("Fnu") + std::to_string(i),
				"erg/s/cm^2/Hz",
				[freddi, lambda]() { return freddi->flux(lambda); }
		});
	}
	return fields;
}

std::vector<FileOutputLongField> FreddiFileOutput::initializeLongFields(const std::shared_ptr<FreddiEvolution>& freddi) {
	return {
			{"h", "cm^2/s", std::bind(&FreddiEvolution::h, freddi)},
			{"R", "cm", std::bind(&FreddiEvolution::R, freddi)},
			{"F", "dyn*cm", std::bind(&FreddiEvolution::F, freddi)},
			{"Sigma", "g/cm^2", std::bind(&FreddiEvolution::Sigma, freddi)},
			{"Teff", "K", std::bind(&FreddiEvolution::Tph, freddi)},
			{"Tvis", "K", std::bind(&FreddiEvolution::Tph_vis, freddi)},
			{"Tirr", "K", std::bind(&FreddiEvolution::Tirr, freddi)},
			{"Height", "cm", std::bind(&FreddiEvolution::Height, freddi)},
	};
}


std::vector<FileOutputShortField> FreddiNeutronStarFileOutput::initializeShortFields(const std::shared_ptr<FreddiNeutronStarEvolution>& freddi) {
	std::vector<FileOutputShortField> fields {
			{"t", "days", [freddi]() {return sToDay(freddi->t());}},
			{"Mdot", "g/s",  std::bind(&FreddiEvolution::Mdot_in, freddi)},
			{"Mdisk", "g", std::bind(&FreddiEvolution::Mdisk, freddi)},
			{"Rhot", "Rsun", [freddi]() {return cmToSun(freddi->R()[freddi->last()]);}},
			{"Cirrout", "float", [freddi]() {return freddi->Cirr()[freddi->last()];}},
			{"H2R", "float", [freddi]() {return freddi->Height()[freddi->last()] / freddi->R()[freddi->last()];}},
			{"Teffout", "K", [freddi]() {return freddi->Tph()[freddi->last()];}},
			{"Tirrout", "K", [freddi]() {return freddi->Tirr()[freddi->last()];}},
			{"Qiir2Qvisout", "float", [freddi]() {return pow(freddi->Tirr()[freddi->last()] / freddi->Tph_vis()[freddi->last()], 4.);}},
			{"Lx", "erg/s", std::bind(&FreddiEvolution::Lx, freddi)},
			{"mU", "mag", std::bind(&FreddiEvolution::mU, freddi)},
			{"mB", "mag", std::bind(&FreddiEvolution::mB, freddi)},
			{"mV", "mag", std::bind(&FreddiEvolution::mV, freddi)},
			{"mR", "mag", std::bind(&FreddiEvolution::mR, freddi)},
			{"mI", "mag", std::bind(&FreddiEvolution::mI, freddi)},
			{"mJ", "mag", std::bind(&FreddiEvolution::mJ, freddi)},
	};
	const auto& lambdas = freddi->args().flux->lambdas;
	for (size_t i = 0; i < lambdas.size(); ++i) {
		const double lambda = lambdas[i];
		fields.push_back(FileOutputShortField{
				std::string("Fnu") + std::to_string(i),
				"erg/s/cm^2/Hz",
				[freddi, lambda]() { return freddi->flux(lambda); }
		});
	}
	return fields;
}

std::vector<FileOutputLongField> FreddiNeutronStarFileOutput::initializeLongFields(const std::shared_ptr<FreddiNeutronStarEvolution>& freddi) {
	return {
			{"h", "cm^2/s", std::bind(&FreddiEvolution::h, freddi)},
			{"R", "cm", std::bind(&FreddiEvolution::R, freddi)},
			{"F", "dyn*cm", std::bind(&FreddiEvolution::F, freddi)},
			{"Sigma", "g/cm^2", std::bind(&FreddiEvolution::Sigma, freddi)},
			{"Teff", "K", std::bind(&FreddiEvolution::Tph, freddi)},
			{"Tvis", "K", std::bind(&FreddiEvolution::Tph_vis, freddi)},
			{"Tirr", "K", std::bind(&FreddiEvolution::Tirr, freddi)},
			{"Height", "cm", std::bind(&FreddiEvolution::Height, freddi)},
	};
}
