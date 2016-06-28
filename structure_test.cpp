/**
 * @file   structure_test.cpp
 * @author Michael Gauland <michael.gauland@canterbury.ac.nz>
 * @date   Mon Nov 16 15:29:29 2015
 * 
 * @brief Unit tests for classes declared in structure.h.
 * 
 * This file part of SLAT (the Seismic Loss Assessment Tool).
 *
 * ©2015 Canterbury University
 */

#include "structure.h"
#include "lognormaldist.h"
#include "relationships.h"
#include "functions.h"
#include "comp_group.h"
#include <memory>
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace std;
using namespace SLAT;

std::shared_ptr<IM> CreateIM(void)
{
    static std::shared_ptr<NonLinearHyperbolicLaw> curve = std::make_shared<NonLinearHyperbolicLaw>(1221, 29.8, 62.2);
    static std::shared_ptr<IM> im = std::make_shared<IM>(curve);
    return im;
}

std::shared_ptr<EDP> CreateEDP(double a)
{
    shared_ptr<DeterministicFn> mu_edp(
        new PowerLawParametricCurve(a, 1.5));
    shared_ptr<DeterministicFn> sigma_edp(
        new PowerLawParametricCurve(0.5, 0.0));
    shared_ptr<ProbabilisticFn> edp_im_relationship(
        new LogNormalFn(mu_edp, LogNormalFn::MEAN_X, sigma_edp, LogNormalFn::SIGMA_LN_X));

    std::shared_ptr<EDP> rel(new EDP(CreateIM(), edp_im_relationship));

    return rel;
}

std::shared_ptr<CompGroup> CreateCG(double a, double count)
{
    std::shared_ptr<FragilityFn> fragFn(new FragilityFn(
                                            { LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(0.0062, 0.4),
                                                    LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(0.0230, 0.4),
                                                    LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(0.0440, 0.4),
                                                    LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(0.0564, 0.4)}));
    std::shared_ptr<LossFn> lossFn(new LossFn(
                                       { LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(0.03, 0.4),
                                               LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(0.08, 0.4),
                                               LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(0.25, 0.4),
                                               LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(1.00, 0.4)}));
    
    return std::make_shared<CompGroup>(CreateEDP(a), fragFn, lossFn, count);
}


BOOST_AUTO_TEST_CASE(structure_empty)
{
    Structure s;

    s.Loss(0.9, false).get_mean_X();
    s.Loss(0.9, true).get_mean_X();
}


BOOST_AUTO_TEST_CASE(structure_just_rb_cost)
{
    Structure s;

    s.setRebuildCost(LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(100, 0.1));
    s.Loss(0.9, false).get_mean_X();
    s.Loss(0.9, true).get_mean_X();
}

BOOST_AUTO_TEST_CASE(structure_one_cg)
{
    Structure s;
    
    std::shared_ptr<IM> im = CreateIM();


    s.setRebuildCost(LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(20, 0.35));
    s.AddCompGroup(CreateCG(0.1, 1));
    s.AddCompGroup(CreateCG(0.05, 5));
    s.AddCompGroup(CreateCG(0.03, 10));
    CreateIM()->SetCollapse(std::make_shared<Collapse>(0.9, 0.470));

    struct {double im; double mean_x; double sd_ln_x; } c_data[] = {
        {0.01, 1.08695e-12, 4.9203}, {0.0225758, 6.18839e-08, 3.64904},
        {0.0351515, 5.58884e-06, 2.99089}, {0.0477273, 7.12212e-05, 2.57756},
        {0.060303, 0.000373696, 2.31591}, {0.0728788, 0.00121206, 2.16254},
        {0.0854545, 0.00294292, 2.08112}, {0.0980303, 0.00592672, 2.04142},
        {0.110606, 0.0105111, 2.02321}, {0.123182, 0.0170405, 2.01477},
        {0.135758, 0.0258778, 2.00966}, {0.148333, 0.0374294, 2.00442},
        {0.160909, 0.0521668, 1.99719}, {0.173485, 0.0706428, 1.98702},
        {0.186061, 0.0934981, 1.97354}, {0.198636, 0.121459, 1.95669},
        {0.211212, 0.155328, 1.93663}, {0.223788, 0.195965, 1.91365},
        {0.236364, 0.244263, 1.88814}, {0.248939, 0.301126, 1.86047},
        {0.261515, 0.367439, 1.83105}, {0.274091, 0.444048, 1.80025},
        {0.286667, 0.531731, 1.76842}, {0.299242, 0.631182, 1.73584},
        {0.311818, 0.742998, 1.7028}, {0.324394, 0.867661, 1.66949},
        {0.33697, 1.00553, 1.63612}, {0.349545, 1.15686, 1.60284},
        {0.362121, 1.32176, 1.56976}, {0.374697, 1.50021, 1.537},
        {0.387273, 1.69211, 1.50463}, {0.399848, 1.89722, 1.47271},
        {0.412424, 2.1152, 1.4413}, {0.425, 2.34562, 1.41043},
        {0.437576, 2.58797, 1.38013}, {0.450152, 2.84167, 1.35043},
        {0.462727, 3.10606, 1.32132}, {0.475303, 3.38044, 1.29283},
        {0.487879, 3.66406, 1.26496}, {0.500455, 3.95616, 1.2377},
        {0.51303, 4.25593, 1.21106}, {0.525606, 4.56255, 1.18504},
        {0.538182, 4.87521, 1.15962}, {0.550758, 5.19307, 1.13481},
        {0.563333, 5.51534, 1.11059}, {0.575909, 5.84119, 1.08696},
        {0.588485, 6.16985, 1.0639}, {0.601061, 6.50056, 1.04141},
        {0.613636, 6.83257, 1.01948}, {0.626212, 7.16518, 0.998102},
        {0.638788, 7.4977, 0.977262}, {0.651364, 7.82951, 0.956952},
        {0.663939, 8.15998, 0.937163}, {0.676515, 8.48854, 0.917884},
        {0.689091, 8.81466, 0.899107}, {0.701667, 9.13784, 0.880822},
        {0.714242, 9.45762, 0.863019}, {0.726818, 9.77358, 0.84569},
        {0.739394, 10.0853, 0.828826}, {0.75197, 10.3925, 0.812417},
        {0.764545, 10.6948, 0.796456}, {0.777121, 10.9919, 0.780932},
        {0.789697, 11.2836, 0.765839}, {0.802273, 11.5696, 0.751168},
        {0.814848, 11.8499, 0.73691}, {0.827424, 12.1241, 0.723058},
        {0.84, 12.3922, 0.709604}, {0.852576, 12.6541, 0.696539},
        {0.865152, 12.9097, 0.683856}, {0.877727, 13.1588, 0.671548},
        {0.890303, 13.4016, 0.659607}, {0.902879, 13.6379, 0.648026},
        {0.915455, 13.8677, 0.636797}, {0.92803, 14.0911, 0.625913},
        {0.940606, 14.3081, 0.615366}, {0.953182, 14.5188, 0.605151},
        {0.965758, 14.7231, 0.595259}, {0.978333, 14.9211, 0.585684},
        {0.990909, 15.113, 0.576418}, {1.00348, 15.2988, 0.567456},
        {1.01606, 15.4786, 0.558789}, {1.02864, 15.6525, 0.550413},
        {1.04121, 15.8207, 0.542319}, {1.05379, 15.9831, 0.5345},
        {1.06636, 16.14, 0.526952}, {1.07894, 16.2915, 0.519666},
        {1.09152, 16.4376, 0.512636}, {1.10409, 16.5786, 0.505857},
        {1.11667, 16.7146, 0.49932}, {1.12924, 16.8456, 0.493021},
        {1.14182, 16.9718, 0.486953}, {1.15439, 17.0934, 0.481109},
        {1.16697, 17.2104, 0.475484}, {1.17955, 17.3231, 0.470071},
        {1.19212, 17.4315, 0.464864}, {1.2047, 17.5358, 0.459857},
        {1.21727, 17.6361, 0.455045}, {1.22985, 17.7325, 0.450421},
        {1.24242, 17.8251, 0.44598}, {1.255, 17.9142, 0.441716},
        {1.26758, 17.9997, 0.437623}, {1.28015, 18.0818, 0.433696},
        {1.29273, 18.1607, 0.429929}, {1.3053, 18.2365, 0.426318},
        {1.31788, 18.3092, 0.422856}, {1.33045, 18.3789, 0.419539},
        {1.34303, 18.4459, 0.416362}, {1.35561, 18.5101, 0.413319},
        {1.36818, 18.5717, 0.410406}, {1.38076, 18.6308, 0.407619},
        {1.39333, 18.6875, 0.404952}, {1.40591, 18.7418, 0.402401},
        {1.41848, 18.7939, 0.399962}, {1.43106, 18.8438, 0.39763},
        {1.44364, 18.8917, 0.395402}, {1.45621, 18.9376, 0.393272},
        {1.46879, 18.9815, 0.391238}, {1.48136, 19.0237, 0.389296},
        {1.49394, 19.064, 0.387441}, {1.50652, 19.1027, 0.385671},
        {1.51909, 19.1397, 0.383981}, {1.53167, 19.1752, 0.382368},
        {1.54424, 19.2092, 0.38083}, {1.55682, 19.2418, 0.379363},
        {1.56939, 19.273, 0.377963}, {1.58197, 19.3029, 0.376629},
        {1.59455, 19.3315, 0.375357}, {1.60712, 19.3589, 0.374144},
        {1.6197, 19.3851, 0.372989}, {1.63227, 19.4103, 0.371887},
        {1.64485, 19.4343, 0.370838}, {1.65742, 19.4574, 0.369838},
        {1.67, 19.4795, 0.368886}, {1.68258, 19.5006, 0.367979},
        {1.69515, 19.5209, 0.367115}, {1.70773, 19.5403, 0.366292},
        {1.7203, 19.5588, 0.365509}, {1.73288, 19.5766, 0.364763},
        {1.74545, 19.5937, 0.364053}, {1.75803, 19.61, 0.363377},
        {1.77061, 19.6256, 0.362734}, {1.78318, 19.6406, 0.362122},
        {1.79576, 19.6549, 0.361539}, {1.80833, 19.6687, 0.360984},
        {1.82091, 19.6818, 0.360456}, {1.83348, 19.6944, 0.359954},
        {1.84606, 19.7065, 0.359476}, {1.85864, 19.7181, 0.359021},
        {1.87121, 19.7292, 0.358588}, {1.88379, 19.7398, 0.358176},
        {1.89636, 19.75, 0.357784}, {1.90894, 19.7598, 0.357411},
        {1.92152, 19.7691, 0.357057}, {1.93409, 19.7781, 0.356719},
        {1.94667, 19.7867, 0.356398}, {1.95924, 19.7949, 0.356093},
        {1.97182, 19.8028, 0.355802}, {1.98439, 19.8104, 0.355525},
        {1.99697, 19.8176, 0.355262}, {2.00955, 19.8246, 0.355012},
        {2.02212, 19.8313, 0.354774}, {2.0347, 19.8377, 0.354548},
        {2.04727, 19.8438, 0.354332}, {2.05985, 19.8497, 0.354127},
        {2.07242, 19.8554, 0.353932}, {2.085, 19.8608, 0.353746},
        {2.09758, 19.866, 0.353569}, {2.11015, 19.871, 0.353401},
        {2.12273, 19.8758, 0.353241}, {2.1353, 19.8804, 0.353089},
        {2.14788, 19.8848, 0.352944}, {2.16045, 19.889, 0.352806},
        {2.17303, 19.8931, 0.352675}, {2.18561, 19.897, 0.35255},
        {2.19818, 19.9008, 0.352431}, {2.21076, 19.9044, 0.352318},
        {2.22333, 19.9078, 0.352211}, {2.23591, 19.9111, 0.352108},
        {2.24848, 19.9143, 0.35201}, {2.26106, 19.9174, 0.351918},
        {2.27364, 19.9204, 0.351829}, {2.28621, 19.9232, 0.351745},
        {2.29879, 19.9259, 0.351665}, {2.31136, 19.9285, 0.351588},
        {2.32394, 19.9311, 0.351516}, {2.33652, 19.9335, 0.351446},
        {2.34909, 19.9358, 0.35138}, {2.36167, 19.9381, 0.351318},
        {2.37424, 19.9402, 0.351258}, {2.38682, 19.9423, 0.351201},
        {2.39939, 19.9443, 0.351147}, {2.41197, 19.9462, 0.351095},
        {2.42455, 19.948, 0.351045}, {2.43712, 19.9498, 0.350999},
        {2.4497, 19.9515, 0.350954}, {2.46227, 19.9532, 0.350911},
        {2.47485, 19.9548, 0.35087}, {2.48742, 19.9563, 0.350832},
        {2.5, 19.9578, 0.350795}};

    for (size_t i=0; i < sizeof(c_data)/sizeof(c_data[0]); i++) {
        LogNormalDist dist = s.Loss(c_data[i].im, true);
        BOOST_CHECK_CLOSE( c_data[i].mean_x, dist.get_mean_X(), 0.1);
        BOOST_CHECK_CLOSE( c_data[i].sd_ln_x, dist.get_sigma_lnX(), 0.1);
    }


    struct {double im; double mean_x; double sd_ln_x; } nc_data[] = {
        {0.01, 1.08695e-12, 4.9203}, {0.0225758, 6.18837e-08, 3.64867},
        {0.0351515, 5.58857e-06, 2.98666}, {0.0477273, 7.12031e-05, 2.55601},
        {0.060303, 0.000373349, 2.24951}, {0.0728788, 0.00120889, 2.01904},
        {0.0854545, 0.00292489, 1.83813}, {0.0980303, 0.00585289, 1.69096},
        {0.110606, 0.0102728, 1.56769}, {0.123182, 0.0163977, 1.46215},
        {0.135758, 0.0243702, 1.37044}, {0.148333, 0.0342652, 1.29013},
        {0.160909, 0.0460978, 1.21968}, {0.173485, 0.0598356, 1.15814},
        {0.186061, 0.0754124, 1.10485}, {0.198636, 0.092742, 1.05932},
        {0.211212, 0.111732, 1.02113}, {0.223788, 0.132293, 0.989854},
        {0.236364, 0.154349, 0.965014}, {0.248939, 0.177841, 0.946066},
        {0.261515, 0.202732, 0.932398}, {0.274091, 0.229006, 0.923343},
        {0.286667, 0.256671, 0.918206}, {0.299242, 0.285752, 0.916289},
        {0.311818, 0.316295, 0.916919}, {0.324394, 0.348361, 0.919476},
        {0.33697, 0.382023, 0.923401}, {0.349545, 0.417363, 0.928211},
        {0.362121, 0.454471, 0.9335}, {0.374697, 0.493439, 0.938934},
        {0.387273, 0.534363, 0.944248}, {0.399848, 0.577338, 0.949237},
        {0.412424, 0.622454, 0.953749}, {0.425, 0.669802, 0.957677},
        {0.437576, 0.719463, 0.960949}, {0.450152, 0.771516, 0.963523},
        {0.462727, 0.82603, 0.965379}, {0.475303, 0.883067, 0.966517},
        {0.487879, 0.942683, 0.96695}, {0.500455, 1.00492, 0.9667},
        {0.51303, 1.06982, 0.965799}, {0.525606, 1.13741, 0.96428},
        {0.538182, 1.20771, 0.962183}, {0.550758, 1.28074, 0.959547},
        {0.563333, 1.35648, 0.956412}, {0.575909, 1.43495, 0.95282},
        {0.588485, 1.51612, 0.948808}, {0.601061, 1.59998, 0.944415},
        {0.613636, 1.6865, 0.939677}, {0.626212, 1.77564, 0.934628},
        {0.638788, 1.86736, 0.929301}, {0.651364, 1.96163, 0.923727},
        {0.663939, 2.05837, 0.917933}, {0.676515, 2.15754, 0.911946},
        {0.689091, 2.25908, 0.90579}, {0.701667, 2.36291, 0.899488},
        {0.714242, 2.46896, 0.893061}, {0.726818, 2.57717, 0.886527},
        {0.739394, 2.68745, 0.879905}, {0.75197, 2.79972, 0.87321},
        {0.764545, 2.9139, 0.866457}, {0.777121, 3.0299, 0.85966},
        {0.789697, 3.14763, 0.852829}, {0.802273, 3.26701, 0.845978},
        {0.814848, 3.38795, 0.839114}, {0.827424, 3.51036, 0.832248},
        {0.84, 3.63414, 0.825388}, {0.852576, 3.7592, 0.81854},
        {0.865152, 3.88546, 0.811713}, {0.877727, 4.01281, 0.80491},
        {0.890303, 4.14117, 0.798139}, {0.902879, 4.27045, 0.791403},
        {0.915455, 4.40056, 0.784706}, {0.92803, 4.53141, 0.778054},
        {0.940606, 4.66291, 0.771448}, {0.953182, 4.79498, 0.764892},
        {0.965758, 4.92753, 0.758388}, {0.978333, 5.06047, 0.751938},
        {0.990909, 5.19373, 0.745546}, {1.00348, 5.32723, 0.739211},
        {1.01606, 5.46088, 0.732931}, {1.02864, 5.59462, 0.726722},
        {1.04121, 5.72836, 0.72057}, {1.05379, 5.86205, 0.71448},
        {1.06636, 5.99559, 0.708454}, {1.07894, 6.12894, 0.702491},
        {1.09152, 6.26202, 0.696593}, {1.10409, 6.39476, 0.69076},
        {1.11667, 6.52712, 0.684991}, {1.12924, 6.65902, 0.679286},
        {1.14182, 6.79042, 0.673647}, {1.15439, 6.92126, 0.668072},
        {1.16697, 7.05148, 0.662562}, {1.17955, 7.18104, 0.657116},
        {1.19212, 7.30989, 0.651734}, {1.2047, 7.43799, 0.646416},
        {1.21727, 7.56528, 0.641161}, {1.22985, 7.69173, 0.63597},
        {1.24242, 7.8173, 0.63084}, {1.255, 7.94196, 0.625773},
        {1.26758, 8.06566, 0.620768}, {1.28015, 8.18837, 0.615824},
        {1.29273, 8.31007, 0.61094}, {1.3053, 8.43073, 0.606117},
        {1.31788, 8.55031, 0.601354}, {1.33045, 8.66879, 0.596649},
        {1.34303, 8.78615, 0.592003}, {1.35561, 8.90237, 0.587416},
        {1.36818, 9.01742, 0.582886}, {1.38076, 9.13129, 0.578412},
        {1.39333, 9.24396, 0.573995}, {1.40591, 9.35542, 0.569634},
        {1.41848, 9.46564, 0.565328}, {1.43106, 9.57462, 0.561077},
        {1.44364, 9.68235, 0.556881}, {1.45621, 9.78882, 0.552737},
        {1.46879, 9.89401, 0.548647}, {1.48136, 9.99793, 0.54461},
        {1.49394, 10.1006, 0.540624}, {1.50652, 10.2019, 0.53669},
        {1.51909, 10.3019, 0.532807}, {1.53167, 10.4007, 0.528974},
        {1.54424, 10.4981, 0.525192}, {1.55682, 10.5943, 0.521458},
        {1.56939, 10.6891, 0.517774}, {1.58197, 10.7827, 0.514138},
        {1.59455, 10.875, 0.51055}, {1.60712, 10.9659, 0.50701},
        {1.6197, 11.0556, 0.503516}, {1.63227, 11.144, 0.500069},
        {1.64485, 11.2311, 0.496668}, {1.65742, 11.3169, 0.493312},
        {1.67, 11.4015, 0.490001}, {1.68258, 11.4847, 0.486735},
        {1.69515, 11.5668, 0.483513}, {1.70773, 11.6476, 0.480335},
        {1.7203, 11.7271, 0.4772}, {1.73288, 11.8054, 0.474107},
        {1.74545, 11.8825, 0.471057}, {1.75803, 11.9584, 0.468049},
        {1.77061, 12.0331, 0.465082}, {1.78318, 12.1066, 0.462157},
        {1.79576, 12.1789, 0.459272}, {1.80833, 12.25, 0.456427},
        {1.82091, 12.3199, 0.453622}, {1.83348, 12.3888, 0.450856},
        {1.84606, 12.4565, 0.448129}, {1.85864, 12.523, 0.445441},
        {1.87121, 12.5885, 0.442791}, {1.88379, 12.6528, 0.440179},
        {1.89636, 12.7161, 0.437604}, {1.90894, 12.7782, 0.435066},
        {1.92152, 12.8394, 0.432564}, {1.93409, 12.8994, 0.430099},
        {1.94667, 12.9585, 0.427669}, {1.95924, 13.0165, 0.425275},
        {1.97182, 13.0735, 0.422915}, {1.98439, 13.1295, 0.420591},
        {1.99697, 13.1845, 0.4183}, {2.00955, 13.2385, 0.416044},
        {2.02212, 13.2916, 0.413821}, {2.0347, 13.3437, 0.411632},
        {2.04727, 13.3949, 0.409474}, {2.05985, 13.4452, 0.407349},
        {2.07242, 13.4946, 0.405256}, {2.085, 13.5431, 0.403195},
        {2.09758, 13.5907, 0.401165}, {2.11015, 13.6374, 0.399166},
        {2.12273, 13.6833, 0.397198}, {2.1353, 13.7284, 0.39526},
        {2.14788, 13.7726, 0.393352}, {2.16045, 13.816, 0.391473},
        {2.17303, 13.8586, 0.389624}, {2.18561, 13.9004, 0.387803},
        {2.19818, 13.9414, 0.386011}, {2.21076, 13.9817, 0.384247},
        {2.22333, 14.0212, 0.382511}, {2.23591, 14.0599, 0.380802},
        {2.24848, 14.098, 0.379121}, {2.26106, 14.1353, 0.377466},
        {2.27364, 14.1719, 0.375838}, {2.28621, 14.2079, 0.374236},
        {2.29879, 14.2431, 0.372659}, {2.31136, 14.2777, 0.371108},
        {2.32394, 14.3116, 0.369583}, {2.33652, 14.3449, 0.368082},
        {2.34909, 14.3775, 0.366605}, {2.36167, 14.4095, 0.365153},
        {2.37424, 14.4409, 0.363725}, {2.38682, 14.4717, 0.36232},
        {2.39939, 14.5019, 0.360939}, {2.41197, 14.5315, 0.35958},
        {2.42455, 14.5605, 0.358244}, {2.43712, 14.589, 0.356931},
        {2.4497, 14.617, 0.355639}, {2.46227, 14.6443, 0.354369},
        {2.47485, 14.6712, 0.353121}, {2.48742, 14.6975, 0.351893},
        {2.5, 14.7233, 0.350687}};

    for (size_t i=0; i < sizeof(nc_data)/sizeof(nc_data[0]); i++) {
        LogNormalDist dist = s.Loss(nc_data[i].im, false);
        BOOST_CHECK_CLOSE( nc_data[i].mean_x, dist.get_mean_X(), 0.1);
        BOOST_CHECK_CLOSE( nc_data[i].sd_ln_x, dist.get_sigma_lnX(), 0.1);
    }
    
}