#include <algorithm>  // transform
#include <vector>

#include "options.hpp"

namespace po = boost::program_options;


GeneralOptions::GeneralOptions(const po::variables_map& vm):
		GeneralArguments(
				vm["prefix"].as<std::string>(),
				vm["dir"].as<std::string>(),
				(vm.count("fulldata") > 0)) {}

po::options_description GeneralOptions::description() {
	po::options_description od("General options");
	od.add_options()
			( "help,h", "Produce help message" )
			( "config", po::value<std::string>(), "Set additional configuration filepath" )
			( "prefix", po::value<std::string>()->default_value(default_prefix), "Set prefix for output filenames. Output file with distribution of parameters over time is PREFIX.dat" )
			( "dir,d", po::value<std::string>()->default_value(default_dir), "Choose the directory to write output files. It should exist" )
			( "fulldata", "Output files PREFIX_%d.dat with radial structure for every time step. Default is to output only PREFIX.dat with global disk parameters for every time step" )
			;
	return od;
}


BasicDiskBinaryOptions::BasicDiskBinaryOptions(const po::variables_map &vm):
		BasicDiskBinaryArguments(
				vm["alpha"].as<double>(),
				sunToGram(vm["Mx"].as<double>()),
				vm["kerr"].as<double>(),
				dayToS(vm["period"].as<double>()),
				sunToGram(vm["Mopt"].as<double>()),
				roptInitializer(vm),
				vm["Topt"].as<double>(),
				rinInitializer(vm),
				routInitializer(vm)) {
	if (rin >= rout) {
		throw po::invalid_option_value("rin should be smaller rout");
	}
}

std::optional<double> BasicDiskBinaryOptions::rinInitializer(const po::variables_map &vm) {
	if (vm.count("rin")) {
		const double Mx = sunToGram(vm["Mx"].as<double>());
		return rgToCm(vm["rin"].as<double>(), Mx);
	}
	return {};
}

std::optional<double> BasicDiskBinaryOptions::routInitializer(const po::variables_map &vm) {
	if (vm.count("rout")) {
		return sunToCm(vm["rout"].as<double>());
	}
	return {};
}

std::optional<double> BasicDiskBinaryOptions::roptInitializer(const po::variables_map &vm) {
	if (vm.count("Ropt")) {
		return sunToCm(vm["Ropt"].as<double>());
	}
	return {};
}

po::options_description BasicDiskBinaryOptions::description() {
	po::options_description od("Basic binary and disk parameter");
	od.add_options()
			( "alpha,a", po::value<double>()->required(), "Alpha parameter of Shakura-Sunyaev model" )
			( "Mx,M", po::value<double>()->required(), "Mass of the central object, in the units of solar masses" )
			( "kerr", po::value<double>()->default_value(default_kerr), "Dimensionless Kerr parameter of the black hole" )
			( "Mopt",	po::value<double>()->required(), "Mass of the optical star, in units of solar masses" )
			( "Ropt", po::value<double>(), "Radius of the optical star, in units of solar radius" )
			( "Topt", po::value<double>()->default_value(default_Topt), "Thermal temperature of the optical star, in units of kelvins" )
			( "period,P", po::value<double>()->required(), "Orbital period of the binary system, in units of days" )
			( "rin", po::value<double>(), "Inner radius of the disk, in the units of the Schwarzschild radius of the central object 2GM/c^2. If it isn't set then the radius of ISCO orbit is used defined by --Mx and --kerr values" )
			( "rout,R", po::value<double>(), "Outer radius of the disk, in units of solar radius. If it isn't set then the tidal radius is used defined by --Mx, --Mopt and --period values" )
			;
	return od;
}


DiskStructureOptions::DiskStructureOptions(const po::variables_map &vm, const BasicDiskBinaryArguments& bdb_args):
		DiskStructureArguments(
				// const BasicDiskBinaryArguments &bdb_args,
				//			const std::string& opacity,
				//			double Mdotout,
				//			const std::string& boundcond, double Thot,
				//			const std::string& initialcond,
				//			std::optional<double> F0,
				//			std::optional<double> Mdisk0, std::optional<double> Mdot0,
				//			std::optional<double> powerorder,
				//			std::optional<double> gaussmu, std::optional<double> gausssigma,
				//			const std::string& wind, const pard& windparams
				bdb_args,
				vm["opacity"].as<std::string>(),
				vm["Mdotout"].as<double>(),
				vm["boundcond"].as<std::string>(),
				vm["Thot"].as<double>(),
				vm["initialcond"].as<std::string>(),
				varToOpt<double>(vm, "F0"),
				varToOpt<double>(vm, "Mdisk0"),
				varToOpt<double>(vm, "Mdot0"),
				varToOpt<double>(vm, "powerorder"),
				varToOpt<double>(vm, "gaussmu"),
				varToOpt<double>(vm, "gausssigma"),
				"no", {}) {}  // wind

po::options_description DiskStructureOptions::description() {
	po::options_description od("Parameters of the disk mode");
	od.add_options()
			( "opacity,O", po::value<std::string>()->default_value(default_opacity), "Opacity law: Kramers (varkappa ~ rho / T^7/2) or OPAL (varkappa ~ rho / T^5/2)" )
			( "Mdotout", po::value<double>()->default_value(default_Mdotout), "Accretion rate onto the disk through its outer radius" )
			( "boundcond", po::value<std::string>()->default_value(default_boundcond), "Outer boundary movement condition\n\n"
																					   "Values:\n"
																					   "  Teff: outer radius of the disk moves inwards to keep photosphere temperature of the disk larger than some value. This value is specified by --Thot option\n"
																					   "  Tirr: outer radius of the disk moves inwards to keep irradiation flux of the disk larger than some value. The value of this minimal irradiation flux is [Stefan-Boltzmann constant] * Tirr^4, where Tirr is specified by --Thot option" ) // fourSigmaCrit, MdotOut
			( "Thot", po::value<double>()->default_value(default_Thot), "Minimum photosphere or irradiation temperature at the outer edge of the hot disk, Kelvin. For details see --boundcond description" )
			( "initialcond", po::value<std::string>()->default_value(default_initialcond), "Type of the initial condition for viscous torque F or surface density Sigma\n\n"
																						   "Values:\n"
																						   "  powerF: F ~ xi^powerorder, powerorder is specified by --powerorder option\n" // power option does the same
																						   "  powerSigma: Sigma ~ xi^powerorder, powerorder is specified by --powerorder option\n"
																						   "  sineF: F ~ sin( xi * pi/2 )\n" // sinus option does the same
																						   "  gaussF: F ~ exp(-(xi-mu)**2 / 2 sigma**2), mu and sigma are specified by --gaussmu and --gausssigma options\n"
																						   "  quasistat: F ~ f(h/h_out) * xi * h_out/h, where f is quasi-stationary solution found in Lipunova & Shakura 2000. f(xi=0) = 0, df/dxi(xi=1) = 0\n\n"
																						   "Here xi is (h - h_in) / (h_out - h_in)\n")
			( "F0", po::value<double>(), "Initial maximum viscous torque in the disk, dyn*cm. Can be overwritten via --Mdisk0 and --Mdot0" )
			( "Mdisk0", po::value<double>(), "Initial disk mass, g. If both --F0 and --Mdisk0 are specified then --Mdisk0 is used. If both --Mdot0 and --Mdisk0 are specified then --Mdot0 is used" )
			( "Mdot0", po::value<double>(), "Initial mass accretion rate through the inner radius, g/s. If --F0, --Mdisk0 and --Mdot0 are specified then --Mdot0 is used. Works only when --initialcond is set to sinusF or quasistat" )
			( "powerorder", po::value<double>(), "Parameter for the powerlaw initial condition distribution. This option works only with --initialcond=powerF or powerSigma" )
			( "gaussmu", po::value<double>(), "Position of the maximum for Gauss distribution, positive number not greater than unity. This option works only with --initialcond=gaussF" )
			( "gausssigma", po::value<double>(), "Width of for Gauss distribution. This option works only with --initialcond=gaussF" )
			;
	return od;
}


SelfIrradiationOptions::SelfIrradiationOptions(const po::variables_map &vm, const DiskStructureArguments &dsa_args):
		SelfIrradiationArguments(
				vm["Cirr"].as<double>(),
				vm["irrfactortype"].as<std::string>()) {
	if (Cirr <= 0. && dsa_args.boundcond == "Tirr") {
		throw po::error("Set positive --Cirr when --boundcond=Tirr !");
	}
	if (irrfactortype != "const" && irrfactortype != "square") {
		throw po::invalid_option_value("--irrfactortype has invalid value");
	}
}

po::options_description SelfIrradiationOptions::description() {
	po::options_description od("Parameters of self-irradiation");
	od.add_options()
			( "Cirr", po::value<double>()->default_value(default_Cirr), "Irradiation factor" )
			( "irrfactortype", po::value<std::string>()->default_value(default_irrfactortype), "Type of irradiation factor Cirr\n\n"
																							   "Values:\n"
																							   "  const: doesn't depend on disk shape:\n[rad. flux] = Cirr  L / (4 pi r^2)\n"
																							   "  square: Cirr depends on the disk relative half-thickness:\n[rad. flux] = Cirr (z/r)^2 L / (4 pi r^2)\n\n"
																							   "Here L is bolometric Luminosity:\nL = eta Mdot c^2" )
			;
	return od;
}


FluxOptions::FluxOptions(const po::variables_map &vm):
		FluxArguments(
				vm["colourfactor"].as<double>(),
				kevToHertz(vm["emin"].as<double>()),
				kevToHertz(vm["emax"].as<double>()),
				vm["staralbedo"].as<double>(),
				vm["inclination"].as<double>(),
				kpcToCm(vm["distance"].as<double>()),
				vm.count("colddiskflux") > 0,
				vm.count("starflux") > 0,
				lambdasInitializer(vm),
				passbandsInitializer(vm)) {}

vecd FluxOptions::lambdasInitializer(const po::variables_map &vm) {
	if (vm.count("lambda") == 0) {
		return vecd();
	}
	vecd lambdas(vm["lambda"].as<vecd>());
	transform(lambdas.begin(), lambdas.end(), lambdas.begin(), angstromToCm);
	return lambdas;
}

std::vector<Passband> FluxOptions::passbandsInitializer(const po::variables_map& vm) {
	if (vm.count("passband") == 0) {
		return {};
	}
	auto filepaths = vm["passband"].as<std::vector<std::string>>();
	std::vector<Passband> passbands;
	for (const auto &filepath : filepaths) {
		try {
			passbands.emplace_back(filepath);
		} catch (const std::ios_base::failure& e) {
			throw po::invalid_option_value("Passband file doesn't exist");
		}
	}
	return passbands;
}

po::options_description FluxOptions::description() {
	po::options_description od("Parameters of flux calculation");
	od.add_options()
			( "colourfactor", po::value<double>()->default_value(default_colourfactor), "Colour factor to calculate X-ray flux"  )
			( "emin", po::value<double>()->default_value(hertzToKev(default_emin)), "Minimum energy of X-ray band, keV" )
			( "emax", po::value<double>()->default_value(hertzToKev(default_emax)), "Maximum energy of X-ray band, keV" )
			( "staralbedo", po::value<double>()->default_value(default_star_albedo), "Part of X-ray radiation reflected by optical star, (1 - albedo) heats star's photosphere. Used only when --starflux is specified" )
			( "inclination,i", po::value<double>()->default_value(default_inclination), "Inclination of the system, degrees" )
			( "distance", po::value<double>()->required(), "Distance to the system, kpc" )
			( "colddiskflux", "Add Fnu for cold disk into output file. Default output is for hot disk only" )
			( "starflux", "Add Fnu for optical star into output file. Mx, Mopt and period must be specified, see also Topt and starlod options. Default output is for hot disk only" )
			( "lambda", po::value<vecd>()->multitoken(), "Wavelength to calculate Fnu, Angstrom. You can use this option multiple times. For each lambda one additional column with values of spectral flux density Fnu [erg/s/cm^2/Hz] is produced" )
			( "passband", po::value<std::vector<std::string>>()->multitoken(), "Path of a file containing tabulated passband, the first column for wavelength in Angstrom, the second column for transmission factor, columns should be separated by spaces" )
			;
	return od;
}


CalculationOptions::CalculationOptions(const po::variables_map &vm):
		CalculationArguments(
				dayToS(vm["time"].as<double>()),
				tauInitializer(vm),
				vm["Nx"].as<unsigned int>(),
				vm["gridscale"].as<std::string>(),
				vm["starlod"].as<unsigned int>()) {
	if (gridscale != "log" && gridscale != "linear") {
		throw po::invalid_option_value("Invalid --gridscale value");
	}
}

std::optional<double> CalculationOptions::tauInitializer(const po::variables_map& vm) {
	if (vm.count("tau")) {
		return dayToS(vm["tau"].as<double>());
	}
	return {};
}

po::options_description CalculationOptions::description() {
	po::options_description od("Parameters of disk evolution calculation");
	od.add_options()
			( "time,T", po::value<double>()->required(), "Time interval to calculate evolution, days" )
			( "tau",	po::value<double>(), "Time step, days" )
			( "Nx",	po::value<unsigned int>()->default_value(default_Nx), "Size of calculation grid" )
			( "gridscale", po::value<std::string>()->default_value(default_gridscale), "Type of grid for angular momentum h: log or linear" )
			( "starlod", po::value<unsigned int>()->default_value(default_starlod), "Level of detail of the optical star 3-D model. The optical star is represented by a triangular tile, the number of tiles is 20 * 4^starlod" )
			;
	return od;
}



FreddiOptions::FreddiOptions(const po::variables_map& vm) {
	if ((vm.count("starflux") > 0)
		&& (vm.count("Mx") == 0 || vm.count("Topt") == 0 || vm.count("Mopt") == 0 || vm.count("period") == 0)) {
		throw po::invalid_option_value("--starflux requires --Mx, --Mopt and --period to be specified");
	}

	general.reset(new GeneralOptions(vm));
	basic.reset(new BasicDiskBinaryOptions(vm));
	disk.reset(new DiskStructureOptions(vm, *basic));
	irr.reset(new SelfIrradiationOptions(vm, *disk));
	flux.reset(new FluxOptions(vm));
	calc.reset(new CalculationOptions(vm));
}

po::options_description FreddiOptions::description() {
	po::options_description desc("Freddi: numerical calculation of accretion disk evolution");
	desc.add(GeneralOptions::description());
	desc.add(BasicDiskBinaryOptions::description());
	desc.add(DiskStructureOptions::description());
	desc.add(SelfIrradiationOptions::description());
	desc.add(FluxOptions::description());
	desc.add(CalculationOptions::description());
	return desc;
}
