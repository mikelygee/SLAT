/**
 * @file   maq.h
 * @author Michael Gauland <michael.gauland@canterbury.ac.nz>
 * @date   Mon Nov 16 15:29:29 2015
 * 
 * @brief  Functions for performing fast integration for SLAT.
 * 
 * This file part of SLAT (the Seismic Loss Assessment Tool).
 *
 * ©2015 Canterbury University
 */
#include <iostream>
#include <iomanip>
#include <stack>
#include <cmath>
#include <limits>
#include <omp.h>
#include <boost/log/trivial.hpp>
#include <gsl/gsl_deriv.h>
#include "maq.h"
#include "context.h"

unsigned int max_count = 0;
unsigned int max_bin = 0;
unsigned int max_successful_bin = 0;
unsigned long bin_evals = 0;
unsigned long maq_evals = 0;
size_t calls = 0;
size_t successes = 0;
size_t fails = 0;
size_t nans = 0;
size_t bin_fails = 0;
const size_t N_BINS = 16;
size_t bin_Bins[N_BINS] = {0};

void ResetIntegrationStats()
{
#pragma omp critical 
    {
        max_count = 0;
        max_bin = 0;
        max_successful_bin = 0;
        maq_evals = 0;
        bin_evals = 0;
        calls = 0;
        successes = 0;
        fails = 0;
        nans = 0;
        bin_fails = 0;
        for (size_t i=0; i < N_BINS; i++) {
            bin_Bins[i] = 0;
    }
    }
}

void DumpIntegrationStats()
{
#pragma omp critical
    std::cout << "Max Count: " << max_count << std::endl
              << "Max Bin: " << max_bin << std::endl
              << "Max Successful Bin: " << max_successful_bin << std::endl
              << "Bin Evals: " << bin_evals << std::endl
              << "MAQ Evals: " << maq_evals << std::endl
              << "Total Count: " << (bin_evals + maq_evals) << std::endl
              << "Calls: " << calls << std::endl
              << "Successes: " << successes << std::endl
              << "Fails: " << fails << std::endl
              << "NANs: " << nans << std::endl
              << "Bin_Fails: " << bin_fails << std::endl;

    std::cout << "Bins: ";
    for (size_t i=0; i < N_BINS; i++) {
        std::cout << bin_Bins[i] << "  ";
    }
    std::cout << std::endl;

}

namespace SLAT {
    namespace Integration {

        unsigned int IntegrationSettings::bin_evals = 0;
        IntegrationSettings::METHOD_TYPE  IntegrationSettings::method = DIRECTED;

        
        src::logger_mt IntegrationSettings::settings_logger;
        IntegrationSettings IntegrationSettings::default_settings;
        
        void IntegrationSettings::Reset(void)
        {
            default_settings.tolerance = TOLERANCE_DEFAULT;
            default_settings.max_evals = EVALUATIONS_DEFAULT;
        }

        IntegrationSettings::IntegrationSettings(void)
        {
            tolerance = TOLERANCE_DEFAULT;
            max_evals = EVALUATIONS_DEFAULT;
            parent = NULL;
        }

        IntegrationSettings::IntegrationSettings(const IntegrationSettings *other)
        {
            tolerance = TOLERANCE_UNSPECIFIED;
            max_evals = EVALUATIONS_UNSPECIFIED;
            parent = other;
        }

        const IntegrationSettings *IntegrationSettings::Get_Global_Settings(void) 
        {
            return &IntegrationSettings::default_settings;
        }

        unsigned int IntegrationSettings::Get_Effective_Max_Evals(void) const
        {
            const IntegrationSettings *ptr = this;
            while (ptr->parent != NULL && ptr->max_evals == EVALUATIONS_UNSPECIFIED) {
                ptr = ptr->parent;
            }
            return ptr->max_evals;
        }


        double IntegrationSettings::Get_Effective_Tolerance(void) const
        {
            const IntegrationSettings *ptr = this;
            while (ptr->parent != NULL && ptr->tolerance == TOLERANCE_UNSPECIFIED) {
                ptr = ptr->parent;
            }
            return ptr->tolerance;
        }

        void IntegrationSettings::Override_Tolerance(double tol) 
        {
            tolerance = tol;
        }

        void IntegrationSettings::Override_Max_Evals(unsigned int evals) 
        {
            max_evals = evals;
        }

        void IntegrationSettings::Use_Default_Tolerance() 
        {
            tolerance = TOLERANCE_UNSPECIFIED;
        }

        void IntegrationSettings::Use_Default_Max_Evals() 
        {
            max_evals = EVALUATIONS_UNSPECIFIED;
        }

        void IntegrationSettings::Set_Tolerance(double tol) 
        {
            if (tol > 0.0) {
                default_settings.Override_Tolerance(tol);
            } else {
                BOOST_LOG(settings_logger) << "Can't set tolerance to " << tol
                                           << "; must be > 0.0.";
            }
        }

        void IntegrationSettings::Set_Max_Evals(unsigned int evals)
        {
            if (evals > 0) {
                default_settings.Override_Max_Evals(evals);
            } else {
                BOOST_LOG(settings_logger) << "Can't set max evaluatons to " << evals
                                           << "; must be > 0.0.";
            }
        }

/*
 * Functions for mapping between the x and t domains:
 */
        inline double x_from_t(double t) { return (t==0 ? INFINITY : 1.0/t - 1.0); };
        inline double t_from_x(double x) { return 1.0/(x + 1.0); };

/*
 * Local structure definition for the limits of integration.
 */
        typedef struct { double a, b; unsigned int evaluations; } search_result_t;
    

/*
 * Try to divide the range [0, ∞) to bracket something interesting for integration.
 */
        search_result_t binary_subdivision(std::function<double (double)> f, unsigned int max_evals)
        {
            double a=0; // Maps to x=∞
            double b=1.0; // Maps to x=0
            double c = (a + b)/2.0;

            double fb = f(x_from_t(b));
            double fc = f(x_from_t(c));
    
            unsigned int evaluations = 2;

            if (fb <= std::numeric_limits<double>::epsilon() && 
                fc <= std::numeric_limits<double>::epsilon()) 
            {
                long int intervals = 4;
                while (evaluations < max_evals && fc <= std::numeric_limits<double>::epsilon()) {
                    for (int i=1; i < intervals; i += 2) {
                        c = float(i)/intervals;
                        fc = f(x_from_t(c));
                        
                        evaluations++;

                        if (fc > std::numeric_limits<double>::epsilon()) {
                            a = (float(i) - 1.0) / intervals;
                            b = (float(i) + 1.0) / intervals;
                            break;
                        }
                    }
                    intervals *= 2;
                }

                if (fc <= std::numeric_limits<double>::epsilon()) {
                    return {NAN, NAN, evaluations};
                }

            }
            return {a, b, evaluations};
        }

/*
 * Try to divide the range [0, ∞) to bracket something interesting for integration, searching in the reverse direction.
 */
        search_result_t reverse_binary_subdivision(std::function<double (double)> f, unsigned int max_evals)
        {
            double a=0; // Maps to x=∞
            double b=1.0; // Maps to x=0
            double c = 1.0 - (a + b)/2.0;

            double fb = f(x_from_t(b));
            double fc = f(x_from_t(c));
    
            unsigned int evaluations = 2;

            if (fb <= std::numeric_limits<double>::epsilon() && 
                fc <= std::numeric_limits<double>::epsilon()) 
            {
                long int intervals = 4;
                while (evaluations < max_evals && fc <= std::numeric_limits<double>::epsilon()) {
                    for (int i=1; i < intervals; i += 2) {
                        c = 1.0 - float(i)/intervals;
                        fc = f(x_from_t(c));
                        
                        evaluations++;

                        if (fc > std::numeric_limits<double>::epsilon()) {
                            a = 1.0 - (float(i) + 1.0) / intervals;
                            b = 1.0 - (float(i) - 1.0) / intervals;
                            break;
                        }
                    }
                    intervals *= 2;
                }

                if (fc <= std::numeric_limits<double>::epsilon()) {
                    return {NAN, NAN, evaluations};
                }

            }
    
            return {a, b, evaluations};
        }

/*
 * Try to divide the range [0, ∞) to bracket something interesting for integration, searching in the reverse direction.
 */
        search_result_t low_first_reverse_binary_subdivision(std::function<double (double)> f, unsigned int max_evals)
        {
            double a=0; // Maps to x=∞
            double b=1.0; // Maps to x=0
            double c = 1.0 - (a + b)/2.0;

            double fb = f(x_from_t(b));
            double fc = f(x_from_t(c));
    
            unsigned int evaluations = 2;

            if (fb <= std::numeric_limits<double>::epsilon() && 
                fc <= std::numeric_limits<double>::epsilon()) 
            {
                long int intervals = 4;
                while (evaluations < max_evals/2 && fc <= std::numeric_limits<double>::epsilon()) {
                    for (int i=1; i < intervals/2; i += 2) {
                        c = 1.0 - float(i)/intervals;
                        fc = f(x_from_t(c));
                        
                        evaluations++;

                        if (fc > std::numeric_limits<double>::epsilon()) {
                            a = 1.0 - (float(i) + 1.0) / intervals;
                            b = 1.0 - (float(i) - 1.0) / intervals;
                            break;
                        }
                    }
                    intervals *= 2;
                }

                intervals = 4;
                while (evaluations < max_evals && fc <= std::numeric_limits<double>::epsilon()) {
                    for (int i=1 + intervals/2; i < intervals; i += 2) {
                        c = 1.0 - float(i)/intervals;
                        fc = f(x_from_t(c));
                        
                        evaluations++;

                        if (fc > std::numeric_limits<double>::epsilon()) {
                            a = 1.0 - (float(i) + 1.0) / intervals;
                            b = 1.0 - (float(i) - 1.0) / intervals;
                            break;
                        }
                    }
                    intervals *= 2;
                }
                if (fc <= std::numeric_limits<double>::epsilon()) {
                    return {NAN, NAN, evaluations};
                }

            }
    
            return {a, b, evaluations};
        }

/*
 * Try to divide the range [0, ∞) to bracket something interesting for integration, alternate method
 */
        search_result_t scattered_search(std::function<double (double)> f, unsigned int max_evals)
        {
            double a=0; // Maps to x=∞
            double b=1.0; // Maps to x=0
            double c = (a + b)/2.0;

            double fb = f(x_from_t(b));
            double fc = f(x_from_t(c));
    
            unsigned int evaluations = 2;

            if (fb <= std::numeric_limits<double>::epsilon() && 
                fc <= std::numeric_limits<double>::epsilon()) 
            {
                unsigned int init_intervals = 4;
                unsigned long long int intervals = init_intervals;
                long int k = 0;
                unsigned int init_i = 1;
                while (evaluations < max_evals && fc <= std::numeric_limits<double>::epsilon()) {
                    unsigned long long int i = init_i;
                    c = 1.0 - float(i)/intervals;
                    fc = f(x_from_t(c));
                    evaluations++;

                    if (fc > std::numeric_limits<double>::epsilon()) {
                        break;
                    }
                    long int p = 2 + 2 * k;
                    k = k + 2;
                    i = i + p;
                    while (i < intervals && evaluations < max_evals) {
                        c = 1.0 - float(i)/intervals;
                        fc = f(x_from_t(c));
                        evaluations++;

                        if (fc > std::numeric_limits<double>::epsilon()) {
                            break;
                        }
                        p = p * 2;
                        i = i + p;
                    }
                    intervals *= 2;

                    if (intervals * 2 == 0) {
                        if (init_i == 0) {
                            init_i +=  2;
                            if (init_i > init_intervals) {
                                init_intervals *= 2;
                            }
                        }
                        intervals = init_intervals;;
                    }
                }
                if (c < 0.5) {
                    a = 0.0;
                    b = 2.0 * c;
                } else {
                    b = 1.0;
                    a = 2.0 * c - 1.0;
                }

                if (fc <= std::numeric_limits<double>::epsilon()) {
                    return {NAN, NAN, evaluations};
                }

            }
            return {a, b, evaluations};
        }

/*
 * Try to divide the range [0, ∞) to bracket something interesting for integration, alternate method #2
 */
        search_result_t directed_search(std::function<double (double)> f, unsigned int max_evals)
        {
            double a=0; // Maps to x=∞
            double b=1.0; // Maps to x=0
            double c = (a + b)/2.0;

            double fb = f(x_from_t(b));
            double fc = f(x_from_t(c));
    
            unsigned int evaluations = 2;

            if (fb <= std::numeric_limits<double>::epsilon() && 
                fc <= std::numeric_limits<double>::epsilon()) 
            {
                while (fc == 0.0 && evaluations < max_evals) {
                    unsigned int init_intervals = 4;
                    unsigned long long int intervals = init_intervals;
                    long int k = 0;
                    unsigned int init_i = 1;
                    while (evaluations < max_evals && fc == 0.0) {
                        unsigned long long int i = init_i;
                        c = 1.0 - float(i)/intervals;
                        fc = f(x_from_t(c));
                        evaluations++;
                        
                        if (fc > 0.0) {
                            break;
                        }
                        long int p = 2 + 2 * k;
                        k = k + 2;
                        i = i + p;
                        while (i < intervals && evaluations < max_evals) {
                            c = 1.0 - float(i)/intervals;
                            fc = f(x_from_t(c));
                            evaluations++;
                            
                            if (fc > 0.0) {
                                break;
                            }
                            p = p * 2;
                            i = i + p;
                        }
                        intervals *= 2;
                        
                        if (intervals * 2 == 0) {
                            if (init_i == 0) {
                                init_i +=  2;
                                if (init_i > init_intervals) {
                                    init_intervals *= 2;
                                }
                            }
                            intervals = init_intervals;;
                        }
                    }
                }

                if (fc == 0.0) {
                    return {NAN, NAN, evaluations};
                }
                    
                if (c < 0.5) {
                    a = 0.0;
                    b = 2.0 * c;
                } else {
                    b = 1.0;
                    a = 2.0 * c - 1.0;
                }

                    
                while (fc <= std::numeric_limits<double>::epsilon() && 
                       evaluations < max_evals &&
                       (b - a > std::numeric_limits<double>::epsilon())) 
                {
                    double d = (a + c) / 2.0;
                    double fd = f(x_from_t(d));
                    evaluations++;

                    if (fd > std::numeric_limits<double>::epsilon()) {
                        c = d;
                        fc = fd;
                        break;
                    } else if (fd <= fc) {
                        a = d;
                    } else {
                        b = c;
                        
                        c = d;
                        fc = fd;
                    }

                    double e = (c + b) / 2.0;
                    double fe = f(x_from_t(e));
                    evaluations++;

                    if (fe > std::numeric_limits<double>::epsilon()) {
                        c = e;
                        fc = fe;
                        break;
                    } else if (fe < fc) {
                        b = e;
                    } else {
                        a = c;
                        
                        c = e;
                        fc = fe;
                    }
                }
            }

            // If the highest value is still too low, act as if we found nothing:
            // if (fc < std::numeric_limits<double>::epsilon()) {
            //     return {NAN, NAN, evaluations};
            // }
            if (c < 0.5) {
                a = 0.0;
                b = 2.0 * c;
            } else {
                b = 1.0;
                a = 2.0 * c - 1.0;
            }
            
            return {a, b, evaluations};
        }
        
/*
 * MAQ integration algorithm
 */
        inline double quad(double a, double b, double c, 
                           double fa, double fb, double fc)
        {
            return (b - a) / 6 * (fa + 4 * fc + fb);
        }

        MAQ_RESULT MAQ(std::function<double (double)> integrand) 
        {
            return MAQ(integrand, *IntegrationSettings::Get_Global_Settings());
        }



        /**
         * @todo Determine how many of the allowed evaluations to use
         * for finding something interesting (and whether it is constant, or
         * programmer-controlled).
         * @todo If binary_subdivision() fails to find something
         * interesting, the programmer should be able to control
         * what happens (treat it as zero, issue a warning and treat
         * it as zero, or fail).
         * @todo Allow programmer to control what happens when number of
         * evaluations is exceeded (warn and use latest approximation, abort).
         */
        MAQ_RESULT MAQ(std::function<double (double)> integrand,
                       const IntegrationSettings &settings)
        {
#pragma omp critical
            calls++;
            
            double tol = settings.Get_Effective_Tolerance();
            unsigned int maxeval = settings.Get_Effective_Max_Evals();
            unsigned int bineval = Integration::IntegrationSettings::bin_evals;
            if (bineval == 0) bineval = maxeval;

            bool success = true;
            // Initialisation
            unsigned int counter;
            double integral = 0.0;

            // Stack for keeping track of what still needs to be evaluated:
            typedef struct {
                double a, b, c;
                double fa, fb, fc; 
                double r;
            } maq_todo;

            std::stack<maq_todo> region_stack;

            /*
             * Determine the range for integration:
             */
            double a, b;
            {
                search_result_t r = {0, 0, 0};
                switch (IntegrationSettings::method) {
                case IntegrationSettings::BINARY_SUBDIVISION:
                    r = binary_subdivision(integrand, bineval);
                    break;
                case IntegrationSettings::REVERSE_BINARY_SUBDIVISION:
                    r = reverse_binary_subdivision(integrand, bineval);
                    break;
                case IntegrationSettings::LOW_FIRST_REVERSE_BINARY_SUBDIVISION:
                    r = low_first_reverse_binary_subdivision(integrand, bineval);
                    break;
                case IntegrationSettings::SCATTERED:
                    r = scattered_search(integrand, bineval);
                    break;
                case IntegrationSettings::DIRECTED:
                    r = directed_search(integrand, bineval);
                    break;
                }
#pragma omp critical
                bin_evals += r.evaluations;
                if (max_bin < r.evaluations) {
#pragma omp critical
                    max_bin = r.evaluations;
                }
                if (max_successful_bin < r.evaluations && r.evaluations < bineval) {
#pragma omp critical
                    max_successful_bin = r.evaluations;
                }
                if (std::isnan(r.a)) {
#pragma omp critical
                    bin_fails++;
                    BOOST_LOG_TRIVIAL(fatal) << Context::GetText() << " binary_subdivision() found nothing";
                    return {0, true, (unsigned int)r.evaluations}; 
                } else {
                    double c = (r.a + r.b) / 2;
                    size_t bin = c * N_BINS;
#pragma omp critical
                    bin_Bins[bin]++;
                }
                a = r.a;
                b = r.b;
                counter = r.evaluations;
            }
   
            double fa = integrand(x_from_t(a))/(a == 0 ? 1 : a*a);
            double fb = integrand(x_from_t(b))/(b*b);

            if (std::isnan(fa)) {
                std::cout << "fa is NAN" << std::endl;
                BOOST_LOG_TRIVIAL(fatal) << Context::GetText() << "; fa is NAN";
#pragma omp critical
                nans++;
#pragma omp critical
                maq_evals += counter;
                return {NAN, false, counter};
            }

            if (std::isnan(fb)) {
                BOOST_LOG_TRIVIAL(fatal) << Context::GetText() << "; fb is NAN";
#pragma omp critical
                nans++;
#pragma omp critical
                maq_evals += counter;
                return {NAN, false, counter};
            }
            
            double c = (a + b)/2.0;
            double fc = integrand(x_from_t(c))/(c*c);
            if (std::isnan(fc)) {
                BOOST_LOG_TRIVIAL(fatal) << Context::GetText() << "; fc is NAN";
#pragma omp critical
                nans++;
#pragma omp critical
                maq_evals += counter;
                return {NAN, false, counter};
            }
            
            counter += 3;

            double r1 = quad(a, b, c, fa, fb, fc);
            region_stack.push({a, b, c, fa, fb, fc, r1});

            double abserr = NAN;
            while (!region_stack.empty()) {
                counter++;
                if (counter > maxeval) {
                    /*
                     * What happens when the number of evaluations is exceeded
                     * without achieving the required accuracy?
                     */
#if false
                    /*
                     * This branch ignores the error, and returns 0
                     */
                    success = true;
                    integral = 0;
                    break;
#elif true
                    /*
                     * This branch generates an error, by returning NAN.
                     */
                    success = false;
                    integral = NAN;
                    break;
#else
                    /*
                     * This branch returns the current approximation, even
                     * though it isn't as good as requested. We should also
                     * generate a warning when this occurs.
                     */
                    success = true;
                    maq_todo todo = region_stack.top();
                    region_stack.pop();
                    integral += todo.r;
                    continue;
#endif
                }
        
                maq_todo todo = region_stack.top();
                region_stack.pop();
                a = todo.a;
                b = todo.b;
                c = todo.c;
        
                fa = todo.fa;
                fb = todo.fb;
                fc = todo.fc;

                r1 = todo.r;

                double d = (a + c)/2.0;
                double e = (b + c)/2.0;
        
                double fd = integrand(x_from_t(d))/(d*d);
                double fe = integrand(x_from_t(e))/(e*e);

        if (std::isnan(fd)) {
            BOOST_LOG_TRIVIAL(fatal) << Context::GetText() << "; fd is NAN";
#pragma omp critical
                    nans++;
#pragma omp critical
                    maq_evals += counter;
                    return {NAN, false, counter};
                }
            
                if (std::isnan(fe)) {
                    BOOST_LOG_TRIVIAL(fatal) << Context::GetText() << "; fe is NAN";
#pragma omp critical
                    nans++;
#pragma omp critical
                    maq_evals += counter;
                    return {NAN, false, counter};
                }
        
                double r2 = quad(a, c, d, fa, fc, fd);
                double r3 = quad(c, b, e, fc, fb, fe);

                double q1 = r1;
                double q2 = r2 + r3;

                abserr = std::abs(q2 - q1);
                if (abserr <= std::abs(tol * q2) || abserr <= std::abs(tol * integral)) {
                    integral = integral + q2 + (q2 - q1)/15;
                } else if (std::abs(r2) > std::abs(r3)) {
                    region_stack.push({c, b, e, fc, fb , fe, r3});
                    region_stack.push({a, c, d, fa, fc, fd, r2});
                } else {
                    region_stack.push({a, c, d, fa, fc, fd, r2});
                    region_stack.push({c, b, e, fc, fb , fe, r3});
                }
            }
            if (counter > max_count) {
#pragma omp critical
                max_count = counter;
            }
            if (success) {
#pragma omp critical
                successes++;
            } else {
#pragma omp critical
                fails++;
            
            }
#pragma omp critical
            maq_evals += counter;
            return {integral, success, counter};
        }
    }
}
