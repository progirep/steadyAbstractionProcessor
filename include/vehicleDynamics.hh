#ifndef __VEHICLE_DYNAMICS_HH__
#define __VEHICLE_DYNAMICS_HH__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <tuple>
#include <vector>
#include <cmath>
#include <functional>
#include "dynamics.hh"
#include "tools.hh"
#include "scots.hh"
#include "RungeKutta4.hh"
#undef fail

class VehicleDynamics : public Dynamics {
private:
public:

    static constexpr unsigned int getStateNofDimensions() {return 4;}
    static constexpr unsigned int getInputNofDimensions() {return 2;}
    static constexpr std::array<bool,4> getDimensionIsTranslationInvariant() {return {true, true, false, false};}
    static constexpr std::array<bool,4> getDimensionIsWrapAround() {return {false, false, true, false};}
    static constexpr std::array<double,4> getDimensionWrapAroundLimits() {return {0.0, 0.0, 2.0*M_PI, 0.0};}

    typedef std::array<double,4> continuous_state_type;
    typedef std::array<double,2> continuous_input_type;
    typedef std::array<int,4> discrete_state_type;
    typedef std::array<int,2> discrete_input_type;

    typedef IntArrayHasher<4> discrete_state_type_hasher;
    typedef IntArrayHasher<2> discrete_input_type_hasher;

    static std::array<std::string,4> getStateVariableNames() {return {"xbit","ybit","curHeading","curSpeed"};}
    static std::array<std::string,2> getInputVariableNames() {return {"rot","acc"};}


    static std::function<void(continuous_state_type &x, continuous_input_type &u)> getSystemDynamicsPost(double tau) {
        return [tau](continuous_state_type &x, continuous_input_type &u)  {

            /* the ode describing the vehicle */
            auto rhs =[](continuous_state_type& xx,  const continuous_state_type &x, continuous_input_type &u) {

                xx[0] = std::sin(x[2])*x[3];
                xx[1] = std::cos(x[2])*x[3];
                xx[2] = u[0];
                xx[3] = u[1];

            };
            scots::runge_kutta_fixed4(rhs,x,u,getStateNofDimensions(),tau,10);
        };
    };

    static std::function<void(continuous_state_type &r, const continuous_state_type &, const continuous_input_type &)> getGrowthBoundFunction(double tau) {
        return [tau](continuous_state_type &r, const continuous_state_type &, const continuous_input_type &) {
            r[0] = r[0] + (3*r[2]+r[3])*tau;
            r[1] = r[1] + (3*r[2]+r[3])*tau;
            r[2] = r[2];
            r[3] = r[3]*0.2;
        };
    }

};


#endif
