#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <nlopt.hpp>

double myvfunc(const std::vector<double> &x, std::vector<double> &grad, void *data)
{
  (void)data;
  if (!grad.empty()) {
    grad[0] = 0.0;
    grad[1] = 0.5 / sqrt(x[1]);
  }
  return sqrt(x[1]);
}

typedef struct {
    double a, b;
} my_constraint_data;

double myvconstraint(const std::vector<double> &x, std::vector<double> &grad, void *data)
{
  my_constraint_data *d = reinterpret_cast<my_constraint_data*>(data);
  double a = d->a, b = d->b;
  if (!grad.empty()) {
    grad[0] = 3 * a * (a*x[0] + b) * (a*x[0] + b);
    grad[1] = -1.0;
  }
  return ((a*x[0] + b) * (a*x[0] + b) * (a*x[0] + b) - x[1]);
}


int main(int argc, char *argv[]) {
  nlopt::opt opt(argc < 2 ? nlopt::LD_MMA : (nlopt::algorithm)atoi(argv[1]), 2);
  const std::vector<double> lb = {-HUGE_VAL, 1e-6};
  opt.set_lower_bounds(lb);
  opt.set_min_objective(myvfunc, NULL);
  my_constraint_data data[2] = { {2,0}, {-1,1} };
  opt.add_inequality_constraint(myvconstraint, &data[0], 1e-8);
  opt.add_inequality_constraint(myvconstraint, &data[1], 1e-8);
  opt.set_xtol_rel(1e-4);

  // try setting an algorithm parameter: */
  opt.set_param("inner_maxeval", 123);
  if (opt.get_param("inner_maxeval", 1234) != 123 || opt.get_param("not a param", 1234) != 1234 ||
      opt.num_params() != 1 || std::string(opt.nth_param(0)) != "inner_maxeval") {
    std::cerr << "failed to retrieve nlopt parameter" << std::endl;
    return EXIT_FAILURE;
  }

  // set a couple of other parameters
  opt.set_param("rho_init", 0.5);
  opt.set_initial_step(0.1);

  std::vector<double> x = {1.234, 5.678};
  double minf = 0.0;

  try{
    opt.optimize(x, minf);
    std::cout << "found minimum at f(" << x[0] << "," << x[1] << ") = "
              << std::setprecision(10) << minf <<std::endl;
    return std::fabs(minf - 0.5443310474) < 1e-3 ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  catch(std::exception &e) {
    std::cerr << "nlopt failed: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
