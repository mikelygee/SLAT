#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <gsl/gsl_statistics.h>
#include <signal.h>
#include "functions.h"
#include "relationships.h"
#include "fragility.h"
#include "loss_functions.h"
#include "maq.h"
#include "comp_group.h"
#include "structure.h"
#include "lognormaldist.h"
#include <chrono>

using namespace std;
using namespace SLAT;

#include <boost/move/utility.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;

int program_status = 0;

void catch_signal(int signal)
{
    cout << "CAUGHT SIGNAL #" << signal << "; status=" << program_status << endl;
    exit(-1);
}

vector<double> logrange(double min, double max, double n)
{
    vector<double> result(n);
    
    for (int i=0; i < n; i++) {
        result[i] = exp(log(min) + i * log(max/min)/(n - 1));
    }    
    return result;
}

vector<double> linrange(double min, double max, double n)
{
    vector<double> result(n);
    
    for (int i=0; i < n; i++) {
        result[i] = min + i * (max - min)/(n - 1);
    }    
    return result;
}

vector<double> frange(double min, double max, double step)
{
    vector<double> result;
  
    double v = min;
    while (v <= max + step/2) {
        result.push_back(v);
        v = v + step;
    }    
    return result;
}

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(main_logger, src::logger_mt)
int main(int argc, char **argv)
{
    signal(SIGSEGV, catch_signal);
    signal(SIGINT, catch_signal);
        
    bool flag = argc > 1;
    int count = 0;
    while (flag || count == 0  ) {
        count++;
        logging::add_file_log("example2.log");
        logging::add_common_attributes();

        src::logger_mt& logger = main_logger::get();
        BOOST_LOG(logger) << "Starting main().";

        cout << "Running Example 2 [" << count << "]" << endl;
        program_status = __LINE__;
        
        // Initialise Caching System:
        Caching::Init_Caching();
        program_status = __LINE__;
    
        // Set up Integration parameters:
        Integration::IntegrationSettings::Set_Tolerance(1E-6);
        Integration::IntegrationSettings::Set_Max_Evals(1024);
    
        // Read IM data
        program_status = __LINE__;
        shared_ptr<IM> im_rel;
        {
            ifstream infile("parser/example2/imfunc.txt");
            char s[512];
            infile.getline(s, sizeof(s));
            if (infile.fail()) {
                cerr << "Error reading imfunc.txt" << endl;
                return -1;
            }
            infile.getline(s, sizeof(s));
            if (infile.fail()) {
                cerr << "Error reading imfunc.txt" << endl;
                return -1;
            }
            vector<double> im, lambda;
            while (!infile.eof()) {
                double x, y;
                infile >> x >> y;
                if (!infile.eof()) {
                    im.insert(im.end(), x);
                    lambda.insert(lambda.end(), y);
                }
            }
            im_rel = make_shared<IM>(make_shared<LogLogInterpolatedFn>(im.data(), lambda.data(), im.size()));
            im_rel->SetCollapse(make_shared<Collapse>(0.9, 0.47));
        }
        program_status = __LINE__;
        
        // Write the IM-RATE relationship:
        {
            vector<double> im_vals = logrange(0.01, 2.5, 199);

            // Calculate the data:
            double results[im_vals.size()];
        
#pragma omp parallel for
            for (size_t i=0; i < im_vals.size(); i++) {
                results[i] = im_rel->lambda(im_vals[i]);
            }

            ofstream ofile("parser/example2/c-results/im_rate");
            ofile << setw(15) << "IM" << setw(15) << "lambda" << endl;
            for (size_t i=0; i < im_vals.size(); i++) 
            {
                ofile << setw(15) << im_vals[i] << setw(15) << results[i] << endl;
            }
        }
        program_status = __LINE__;

        // Write the Collapse data:
        {
            vector<double> im_vals = linrange(0.01, 2.5, 199);
            double results[im_vals.size()];
#pragma omp parallel for
            for (size_t i=0; i < im_vals.size(); i++) {
                results[i] = im_rel->pCollapse(im_vals[i]);
            }
        
            ofstream ofile("parser/example2/c-results/collapse.txt");
            ofile << setw(15) << "IM" << setw(15) << "p(Collapse)" << endl;
            for (size_t i=0; i < im_vals.size(); i++) 
            {
                ofile << setw(15) << im_vals[i] << setw(15) << results[i] << endl;
            }
        }
        program_status = __LINE__;

        // Write the rate of collapse:
        {
            ofstream ofile("parser/example2/c-results/collrate.txt");
            ofile << setw(15) << "Rate of Collapse for IM IM_1 is "
                  << setprecision(16) << im_rel->CollapseRate() << endl;
        }
        program_status = __LINE__;

        // Read in EDP functions
        const int N_EDPS = 21;
        map<int, shared_ptr<EDP>> edp_rels;
        {
            for (int i=0; i < N_EDPS; i++) {
                program_status = __LINE__;
                int n = i + 1;
            
                stringstream path;
                path << "/home/mag109/SLATv1.15_Public/example2_10storeybuilding/RB_EDP" << n << ".txt";

                ifstream infile(path.str());
                char s[512];
                infile.getline(s, sizeof(s));
                if (infile.fail()) {
                    cerr << "Error reading " << path.str() << endl;
                    return -1;
                }
                infile.getline(s, sizeof(s));
                if (infile.fail()) {
                    cerr << "Error reading " << path.str() << endl;
                    return -1;
                }

                vector<double> im_val, mean_edp, sd_edp;
                im_val.push_back(0);
                mean_edp.push_back(0);
                sd_edp.push_back(0);
            
                while (!infile.eof()) {
                    program_status = __LINE__;
                    char line[2048];
                    infile.getline(line, sizeof(line));
                    if (!infile.eof()) {
                        if (infile.fail()) {
                            cerr << "Error reading " << path.str() << endl;
                            //return -1;
                        } else {
                            stringstream s(line);
                            double im;
                            double edp;
                            vector<double> edps;

                            s >> im;
                            if (s.fail()) {
                                break;
                            }
                            im_val.insert(im_val.end(), im);
                        
                            while (!s.eof()) {
                                s >> edp;
                                if (!s.eof() && edp != 0) {
                                    edps.push_back(edp);
                                }
                            }
                            mean_edp.insert(mean_edp.end(), gsl_stats_mean(edps.data(), 1, edps.size()));
                            sd_edp.insert(sd_edp.end(), gsl_stats_sd(edps.data(), 1, edps.size()));
                        }
                    }
                }
                program_status = __LINE__;
                {
                
                    edp_rels[n] = make_shared<EDP>(
                        im_rel, 
                        make_shared<LogNormalFn>(
                            make_shared<LinearInterpolatedFn>(im_val.data(), mean_edp.data(), im_val.size()),
                            LogNormalFn::MEAN_X,
                            make_shared<LinearInterpolatedFn>(im_val.data(), sd_edp.data(), im_val.size()),
                            LogNormalFn::SIGMA_X));
                }
            }
        }
        program_status = __LINE__;

        // Dump EDP-IM relationship:
        for (int i=0; i < N_EDPS; i++) {
            int n = i + 1;
            {
                vector<double> im_vals = linrange(0.01, 2.5, 199);
                double mean[im_vals.size()], sd[im_vals.size()];
            
#pragma omp parallel for
                for (size_t i=0; i < im_vals.size(); i++) {
                    mean[i] = edp_rels[n]->Mean(im_vals[i]);
                    sd[i] = edp_rels[n]->SD_ln(im_vals[i]);
                }
            
                stringstream path;
                path << "parser/example2/c-results/im_edp_" << n << ".txt";
            
                ofstream outfile(path.str());
                outfile << setw(15) << "IM" << setw(15) << "mean_x" << setw(15) << "sd_ln_x" << endl;
            
                for (size_t i=0; i < im_vals.size(); i++) {
                    outfile << setw(15) << im_vals[i]
                            << setw(15) << mean[i]
                            << setw(15) << sd[i] << endl;
                }
            }
            program_status = __LINE__;

            {
                stringstream path;
                path << "parser/example2/c-results/edp_" << n << "_rate.txt";

                ofstream outfile(path.str());
                outfile << setw(15) << "EDP" << setw(15) << "lambda" << endl;
        
                vector<double> edp_vals;
                if (n == 1) {
                    edp_vals = logrange(0.001, 0.15, 149);
                } else if (n == 2) {
                    edp_vals = logrange(0.001, 0.10, 149);
                } else if ((n % 2) == 1) {
                    edp_vals = logrange(0.05, 5.0, 199);
                } else {
                    edp_vals = logrange(0.001, 0.1, 199);
                }
                double results[edp_vals.size()];
#pragma omp parallel for
                for (size_t i=0; i < edp_vals.size(); i++) {
                    results[i] = edp_rels[n]->lambda(edp_vals[i]);
                }
        
                for (size_t i=0; i < edp_vals.size(); i++) {
                    outfile << setw(15) << edp_vals[i] 
                            << setw(15) << results[i] << endl;
                }
            }
            program_status = __LINE__;
        }
    
        // Fragility and Loss functions
        map<int, shared_ptr<FragilityFn>> fragility_functions;
        map<int, shared_ptr<LossFn>> loss_functions;
        {
            struct DATA {double id; vector<pair<double, double>> frag; vector<pair<double, double>> loss;};
            vector<struct DATA> data = {
                {2, {{0.005, 0.40}, {0.010, 0.45}, {0.030, 0.50}, {0.060, 0.60}},
                 {{1143, 0.42}, {3214, 0.40}, {4900, 0.37}, {4900, 0.37}}},
                {3, {{0.004, 0.39}, {0.0095, 0.25}, {0.02, 0.62}, {0.0428, 0.36}},
                 {{590, 0.59}, {2360, 0.63}, {5900, 0.67}, {5900, 0.67}}},
                {105, {{0.0039, 0.17}, {0.0085, 0.23}},
                 {{29.9, 0.2}, {178.7, 0.2}}},
                {107, {{0.04, 0.36}, {0.046, 0.33}},
                 { {131.7, 0.26}, {131.7, 0.26}}},
                {203, {{0.55, 0.4}, {1.0, 0.4}},
                 {{46.73131, 0.4}, {282.4967, 0.4}}},
                {211, {{32, 1.4}},
                 {{900, 1.0}}},
                {208, {{1.2, 0.6}},
                 {{1783.333, 0.4}}},
                {209, {{0.8, 0.5}},
                 {{40000, 0.4}}},
                {205, {{1.6, 0.5}},
                 {{196666.7, 0.6}}},
                {204, {{0.4, 0.3}},
                 {{56000, 0.2}}},
                {106, {{0.0039, 0.17}},
                 {{16.7, 0.2}}},
                {108, {{0.004, 0.5}, {0.008, 0.5}, {0.025, 0.5}, {0.050, 0.5}},
                 {{  250.0, 0.63}, { 1000.0, 0.63}, { 5000.0, 0.63}, {10000.0, 0.63}}},
                {214, {{0.25, 0.6}, {0.50, 0.6}, {1.00, 0.6}, {2.00, 0.6}},
                 {{ 200.0, 0.63}, { 1200.0, 0.63}, { 3600.0, 0.63}, {10000.0, 0.63}}}};

        program_status = __LINE__;
#pragma omp parallel for
            for (size_t i=0; i < data.size(); i++) {
                {
                    vector<LogNormalDist> dists(data[i].frag.size());
                
                    for (size_t j=0; j < data[i].frag.size(); j++) {
                        dists[j] = LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(
                            data[i].frag[j].first, data[i].frag[j].second);
                    }
#pragma omp critical
                    fragility_functions[data[i].id] = make_shared<FragilityFn>(dists);
                }
                {
                    vector<LogNormalDist> dists(data[i].frag.size());
                
                    for (size_t j=0; j < data[i].loss.size(); j++) {
                        dists[j] = LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(
                            data[i].loss[j].first, data[i].loss[j].second);
                    }
#pragma omp critical
                    loss_functions[data[i].id] = make_shared<LossFn>(dists);
                }
            }
        }
        program_status = __LINE__;

        // Component Groups
        map<int, shared_ptr<CompGroup>> compgroups;
        {
            struct DATA {int id, edp, frag, count;};
            vector <struct DATA> data = 
                {
                    {1, 2, 2, 20}, {2, 2, 2, 18}, {3, 4, 2, 4}, {4, 4, 2, 18}, 
                    {5, 6, 2, 4}, {6, 6, 2, 18}, {7, 8, 2, 4}, {8, 8, 2, 18},
                    {9, 10, 2, 4}, {10, 10, 2, 18}, {11, 12, 2, 4}, {12, 12, 2, 18},
                    {13, 14, 2, 4}, {14, 14, 2, 18}, {15, 16, 2, 4}, {16, 16, 2, 18},
                    {17, 18, 2, 4}, {18, 18, 2, 18}, {19, 20, 2, 4}, {20, 20, 2, 18},
                    {21, 2, 3, 16}, {22, 4, 3, 16}, {23, 6, 3, 16}, {24, 8, 3, 16}, 
                    {25, 10, 3, 16}, {26, 12, 3, 16}, {27, 14, 3, 16}, 
                    {28, 16, 3, 16}, {29, 18, 3, 16}, {30, 20, 3, 16}, 
                    {31, 2, 105, 721}, {32, 4, 105, 721}, {33, 6, 105, 721},
                    {34, 8, 105, 721}, {35, 10, 105, 721}, {36, 12, 105, 721}, 
                    {37, 14, 105, 721}, {38, 16, 105, 721}, {39, 18, 105, 721}, 
                    {40, 20, 105, 721}, {41, 2, 107, 99}, {42, 4, 107, 99}, 
                    {43, 6, 107, 99}, {44, 8, 107, 99}, {45, 10, 107, 99},
                    {46, 12, 107, 99}, {47, 14, 107, 99}, {48, 16, 107, 99},
                    {49, 18, 107, 99}, {50, 20, 107, 99}, {51, 3, 203, 693},
                    {52, 5, 203, 693}, {53, 7, 203, 693}, {54, 9, 203, 693},
                    {55, 11, 203, 693}, {56, 13, 203, 693}, {57, 15, 203, 693},
                    {58, 17, 203, 693}, {59, 19, 203, 693}, {60, 21, 203, 693},
                    {61, 3, 211, 23}, {62, 5, 211, 23}, {63, 7, 211, 23}, 
                    {64, 9, 211, 23}, {65, 11, 211, 23}, {66, 13, 211, 23}, 
                    {67, 15, 211, 23}, {68, 17, 211, 23}, {69, 19, 211, 23}, 
                    {70, 21, 211, 23}, {71, 1, 208, 53}, {72, 3, 208, 53}, 
                    {73, 5, 208, 53}, {74, 7, 208, 53}, {75, 9, 208, 53}, 
                    {76, 11, 208, 53}, {77, 13, 208, 53}, {78, 15, 208, 53}, 
                    {79, 17, 208, 53}, {80, 19, 208, 53}, {81, 5, 209, 16}, 
                    {82, 11, 209, 16}, {83, 19, 209, 16}, {84, 21, 205, 4}, 
                    {85, 1, 204, 2}, {86, 2, 106, 721}, {87, 4, 106, 721}, 
                    {88, 6, 106, 721}, {89, 8, 106, 721}, {90, 10, 106, 721},
                    {91, 12, 106, 721}, {92, 14, 106, 721}, {93, 16, 106, 721},
                    {94, 18, 106, 721}, {95, 20, 106, 721}, {96, 2, 108, 10}, 
                    {97, 4, 108, 10}, {98, 6, 108, 10}, {99, 8, 108, 10},
                    {100, 10, 108, 10}, {101, 12, 108, 10}, {102, 14, 108, 10},
                    {103, 16, 108, 10}, {104, 18, 108, 10}, {105, 20, 108, 10}, 
                    {106, 1, 214, 10}, {107, 3, 214, 10}, {108, 5, 214, 10},
                    {109, 7, 214, 10}, {110, 9, 214, 10}, {111, 11, 214, 10}, 
                    {112, 13, 214, 10}, {113, 15, 214, 10}, {114, 17, 214, 10},
                    {115, 19, 214, 10}};

            program_status = __LINE__;
#pragma omp parallel for
            for (size_t i=0; i < data.size(); i++) {
                stringstream name;
                name << "Component Group #" << i;
                shared_ptr<CompGroup> cg = make_shared<CompGroup>(
                    edp_rels[data[i].edp],
                    fragility_functions[data[i].frag],
                    loss_functions[data[i].frag],
                    data[i].count,
                    name.str());
#pragma omp critical
                compgroups[data[i].id] = cg;

                (void)cg->E_annual_loss();
            }
            program_status = __LINE__;
            
            //cout << "post-compgroups " << omp_get_wtime() << endl;
            
            for (map<int, shared_ptr<CompGroup>>::const_iterator i = compgroups.cbegin();
                 i != compgroups.cend();
                 i++)
            {
                program_status = __LINE__;
                int n = i->first;
                shared_ptr<CompGroup> cg = i->second;

                {
                    // Record LOSS-EDP relationship
                    stringstream path;
                    path << "parser/example2/c-results/loss_" << n << "_edp.txt";
            
                    ofstream outfile(path.str());
                    outfile << setw(15) << "EDP" << setw(15) << "mean_x" 
                            << setw(15) << "sd_ln_x" << endl;
        
                    vector<double> edp_vals;

                    if (n == 1 || n == 2 || n == 86 || n == 96) {
                        edp_vals = linrange(0.001, 0.10, 149);
                    } else if (n == 21 || n == 31 || n == 41) {
                        edp_vals = linrange(0.001, 0.10, 149);
                    } else if (n == 71 || n == 85 || n == 106) {
                        edp_vals = linrange(0.001, 0.15, 149);
                    } else if (n == 86 || n == 87) {
                        edp_vals = linrange(0.001, 0.10, 199);
                    } else if ((n >= 51 && n < 87) || (n >= 107)) {
                        edp_vals = linrange(0.05, 5.0, 199);
                    } else {
                        edp_vals = linrange(0.001, 0.10, 199);
                    }

                    //cout << "BEFORE LOSS-EDP " << omp_get_wtime() << endl;
        
                    {
                        double e[edp_vals.size()], sd[edp_vals.size()];
                        for (size_t i=0; i < edp_vals.size(); i++) {
#pragma omp parallel for
                            for (size_t j=0; j < 16; j++) {
                                double edp = edp_vals[i];
                                e[i] = cg->E_loss_EDP(edp);
                                sd[i] = cg->SD_ln_loss_EDP(edp);
                            }
                        }
                    
                        for (size_t i=0; i < edp_vals.size(); i++) {
                            outfile << setw(15) << edp_vals[i]
                                    << setw(15) << e[i]
                                    << setw(15) << sd[i]
                                    << endl;
                        }
                    }
                    //cout << "AFTER LOSS-EDP " << omp_get_wtime() << endl;
                }
                program_status = __LINE__;

                if (false) {
                    // Record LOSS-IM relationship
                    stringstream path;
                    path << "parser/example2/c-results/loss_" << n << "_im.txt";
            
                    ofstream outfile(path.str());
                    outfile << setw(15) << "IM" << setw(15) << "mean_x" 
                            << setw(15) << "sd_ln_x" << endl;
        
                    vector<double> im_vals = linrange(0.01, 2.5, 199);
                    double e[im_vals.size()], sd[im_vals.size()];
                
                    //cout << "BEFORE LOSS-IM " << omp_get_wtime() << endl;
                    program_status = __LINE__;
#pragma omp parallel for
                    for (size_t i=0; i < im_vals.size(); i++) {
                        e[i] = cg->E_loss_IM(im_vals[i]); 
                        sd[i] = cg->SD_ln_loss_IM(im_vals[i]); 
                    }
                    program_status = __LINE__;
                    
                    for (size_t i=0; i < im_vals.size(); i++) {
                        outfile << setw(15) << im_vals[i]
                                << setw(15) << e[i]
                                << setw(15) << sd[i]
                                << endl;
                    }
                    //cout << "AFTER LOSS-IM " << omp_get_wtime() << endl;
                }
                program_status = __LINE__;

                {
                    // Record DS-EDP relationship
                    stringstream path;
                    path << "parser/example2/c-results/ds_edp_" << n << ".txt";

                    shared_ptr<FragilityFn> fragility = cg->FragFn();
                
                    ofstream outfile(path.str());
                    outfile << setw(15) << "EDP";
                    for (size_t i=1; i <= fragility->n_states(); i++) {
                        stringstream label;
                        label << "DS" << i;
                        outfile << setw(15) << label.str();
                    }
                    outfile << endl;
                
                    vector<double> edp_vals = frange(0.0, 0.200, 0.01);
                    vector<double> results[edp_vals.size()];
                
                    //cout << "BEFORE DS-EDP " << omp_get_wtime() << endl;
#pragma omp parallel for
                        for (size_t i=0; i < edp_vals.size(); i++) {
                            vector<double> temp = fragility->pExceeded(edp_vals[i]);
#pragma omp critical
                            results[i] = temp;
                        }

                        for (size_t i=0; i < edp_vals.size(); i++) {
                            outfile << setw(15) << edp_vals[i];
                            for (vector<double>::const_iterator j = results[i].cbegin();
                                 j != results[i].cend();
                                 j++) 
                            {
                                outfile << setw(15) << *j;
                            }
                            outfile << endl;
                        }
                        //cout << "AFTER DS-EDP " << omp_get_wtime() << endl;
                }
                program_status = __LINE__;
                
                {
                    // Record LOSS-RATE relationship
                    stringstream path;
                    path << "parser/example2/c-results/loss_rate_" << n << ".txt";

                    ofstream outfile(path.str());
                    outfile << setw(15) << "t" << setw(15) << "Rate" << endl;
                
                    vector<double> t_vals = frange(1E-4, 1.2, 4.8E-3);
                    double losses[t_vals.size()];
                    //cout << "BEFORE LOSS-RATE " << omp_get_wtime() << endl;
                program_status = __LINE__;
#pragma omp parallel for
                    for (size_t i=0; i < t_vals.size(); i++) {
                        losses[i] = cg->lambda_loss(t_vals[i]);
                    }
                program_status = __LINE__;

                    for (size_t i=0; i < t_vals.size(); i++) {
                        outfile << setw(15) << t_vals[i]
                                << setw(15) << losses[i] << std::endl;
                    }
                    //cout << "AFTER LOSS-RATE " << omp_get_wtime() << endl;
                }
                program_status = __LINE__;

                {
                    // Record Annual Loss relationship
                    stringstream path;
                    path << "parser/example2/c-results/annual_loss_" << n << ".txt";

                    ofstream outfile(path.str());
                    outfile << setw(15) << "t" << setw(15) << "E[ALt]" << endl;
                
                    vector<double> t_vals = frange(1.0, 100.0, 1.0);
                    double losses[t_vals.size()];
                    //cout << "BEFORE ANNUAL LOSS " << omp_get_wtime() << endl;
#pragma omp parallel for
                    for (size_t i=0; i < t_vals.size(); i++) {
                        losses[i] = cg->E_loss(t_vals[i], 0.06);
                    }

                    for (size_t i=0; i < t_vals.size(); i++) {
                        outfile << setw(15) << t_vals[i]
                                << setw(15) << losses[i] << std::endl;
                    }
                    //cout << "AFTER ANNUAL LOSS " << omp_get_wtime() << endl;
                }
            }
            program_status = __LINE__;
        }
        program_status = __LINE__;

        shared_ptr<Structure> building = make_shared<Structure>("building");
        {
            program_status = __LINE__;
            for (map<int, shared_ptr<CompGroup>>::const_iterator i = compgroups.cbegin();
                 i != compgroups.cend();
                 i++)
            {
                building->AddCompGroup(i->second);
            }
        }
        program_status = __LINE__;
        building->setRebuildCost(LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(14E6, 0.35));
        program_status = __LINE__;
                
        {
            // Record the Loss calculations for the structure as a whole, without collapse:
            ofstream outfile("parser/example2/c-results/loss_nc_total");
            outfile << setw(15) << "IM" << setw(15) << "mean_x"
                    << setw(15) << "sd_ln_x" << endl;
                
            vector<double> im_vals = linrange(0.01, 2.5, 199);
                
            for (vector<double>::const_iterator im = im_vals.begin();
                 im != im_vals.end();
                 im++)
            {
                LogNormalDist dist = building->Loss(*im, false);
                outfile << setw(15) << *im << setw(15) << dist.get_mean_X()
                        << setw(15) << dist.get_sigma_lnX()
                        << endl;
            }
        }
        program_status = __LINE__;

        {
            // Record the Loss calculations for the structure as a whole, including collapse:
            ofstream outfile("parser/example2/c-results/loss_c_total");
            outfile << setw(15) << "IM" << setw(15) << "mean_x"
                    << setw(15) << "sd_ln_x" << endl;
        
            vector<double> im_vals = linrange(0.01, 2.5, 199);
                
            for (vector<double>::const_iterator im = im_vals.begin();
                 im != im_vals.end();
                 im++)
            {
                LogNormalDist dist = building->Loss(*im, true);
                outfile << setw(15) << *im << setw(15) << dist.get_mean_X()
                        << setw(15) << dist.get_sigma_lnX()
                        << endl;
            }
        }
        program_status = __LINE__;

        {
            // Record the deaggregated loss for the structure:
            ofstream outfile("parser/example2/c-results/deagg");
            outfile << setw(15) << "IM" 
                    << setw(15) << "mean_nc"
                    << setw(15) << "sd_nc"
                    << setw(15) << "mean_c"
                    << setw(15) << "sd_c" << endl;
        
            vector<double> im_vals = linrange(0.01, 2.5, 199);
                
            for (vector<double>::const_iterator im = im_vals.begin();
                 im != im_vals.end();
                 im++)
            {
                pair<LogNormalDist, LogNormalDist> dists = building->DeaggregatedLoss(*im);

                outfile << setw(15) << *im 
                        << setw(15) << dists.first.get_mean_X()
                        << setw(15) << dists.first.get_sigma_lnX()
                        << setw(15) << dists.second.get_mean_X()
                        << setw(15) << dists.second.get_sigma_lnX()
                        << endl;
            }
        }
        program_status = __LINE__;
        cout << "Done" << endl;
    }
    return 0;
}