#ifndef FREDDI_APPLICATION_H
#define FREDDI_APPLICATION_H

#include <iostream>

#include "options.hpp"
#include "output.hpp"


template <typename Output, typename Options, typename Evolution>
void run_application(int ac, char *av[]) {
	auto vm = parseOptions<Options>(ac, av);
	if (vm.count("help") > 0) {
		std::cout << Options::description() << std::endl;
		return;
	}
	Options opts(vm);
	std::shared_ptr<Evolution> freddi{new Evolution(opts)};
	Output output(freddi, vm);
	for (int i_t = 0; i_t <= static_cast<int>(freddi->args().calc->time / freddi->args().calc->tau); i_t++) {
		output.dump();
		freddi->step();
	}
}

#endif //FREDDI_APPLICATION_H
