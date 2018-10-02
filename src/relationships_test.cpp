/**
 * @file   relationships_test.cpp
 * @author Michael Gauland <michael.gauland@canterbury.ac.nz>
 * @date   Mon Nov 16 15:29:29 2015
 * 
 * @brief Unit tests for classes declared in relationships.h.
 * 
 * This file part of SLAT (the Seismic Loss Assessment Tool).
 *
 * ©2015 Canterbury University
 */
#include "relationships.h"
#include "maq.h"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace SLAT;

BOOST_AUTO_TEST_CASE( Simple_Rate_Relationship_Test )
{
    /*
     * Test against the IM used in the simplified bridge example, as calculated
     * by the original SLAT program.
     */
    shared_ptr<DeterministicFn> im_rate_function(
        new NonLinearHyperbolicLaw(1221, 29.8, 62.2));

    IM im_rate_rel(im_rate_function);

    struct { double im, rate; } test_data[] = {
        { 1.0000E-03, 2.915 }, { 1.0822E-03, 2.782 }, { 1.1712E-03, 2.653 }, { 1.2676E-03, 2.529 },
        { 1.3718E-03, 2.408 }, { 1.4846E-03, 2.291 }, { 1.6067E-03, 2.178 }, { 1.7388E-03, 2.070 },
        { 1.8818E-03, 1.964 }, { 2.0366E-03, 1.863 }, { 2.2041E-03, 1.765 }, { 2.3853E-03, 1.671 },
        { 2.5815E-03, 1.581 }, { 2.7938E-03, 1.494 }, { 3.0235E-03, 1.410 }, { 3.2722E-03, 1.330 },
        { 3.5413E-03, 1.253 }, { 3.8325E-03, 1.179 }, { 4.1477E-03, 1.108 }, { 4.4888E-03, 1.041 },
        { 4.8579E-03, 0.9760 }, { 5.2575E-03, 0.9144 }, { 5.6898E-03, 0.8557 }, { 6.1577E-03, 0.7997 },
        { 6.6641E-03, 0.7464 }, { 7.2122E-03, 0.6958 }, { 7.8053E-03, 0.6477 }, { 8.4472E-03, 0.6021 },
        { 9.1418E-03, 0.5590 }, { 9.8936E-03, 0.5181 }, { 1.0707E-02, 0.4796 }, { 1.1588E-02, 0.4432 },
        { 1.2541E-02, 0.4089 }, { 1.3572E-02, 0.3766 }, { 1.4688E-02, 0.3463 }, { 1.5896E-02, 0.3179 },
        { 1.7203E-02, 0.2913 }, { 1.8618E-02, 0.2664 }, { 2.0149E-02, 0.2431 }, { 2.1806E-02, 0.2215 },
        { 2.3600E-02, 0.2013 }, { 2.5540E-02, 0.1826 }, { 2.7641E-02, 0.1653 }, { 2.9914E-02, 0.1493 },
        { 3.2374E-02, 0.1345 }, { 3.5036E-02, 0.1209 }, { 3.7918E-02, 0.1084 }, { 4.1036E-02, 9.6884E-02 },
        { 4.4411E-02, 8.6390E-02 }, { 4.8063E-02, 7.6816E-02 }, { 5.2015E-02, 6.8103E-02 }, { 5.6293E-02, 6.0196E-02 },
        { 6.0922E-02, 5.3040E-02 }, { 6.5932E-02, 4.6581E-02 }, { 7.1355E-02, 4.0770E-02 }, { 7.7223E-02, 3.5558E-02 },
        { 8.3573E-02, 3.0899E-02 }, { 9.0446E-02, 2.6747E-02 }, { 9.7884E-02, 2.3061E-02 }, { 0.1059, 1.9801E-02 },
        { 0.1146, 1.6928E-02 }, { 0.1241, 1.4407E-02 }, { 0.1343, 1.2203E-02 }, { 0.1453, 1.0286E-02 },
        { 0.1573, 8.6251E-03 }, { 0.1702, 7.1937E-03 }, { 0.1842, 5.9661E-03 }, { 0.1993, 4.9188E-03 },
        { 0.2157, 4.0304E-03 }, { 0.2335, 3.2811E-03 }, { 0.2527, 2.6529E-03 }, { 0.2735, 2.1297E-03 },
        { 0.2960, 1.6968E-03 }, { 0.3203, 1.3413E-03 }, { 0.3466, 1.0514E-03 }, { 0.3751, 8.1701E-04 },
        { 0.4060, 6.2898E-04 }, { 0.4394, 4.7950E-04 }, { 0.4755, 3.6178E-04 }, { 0.5146, 2.6998E-04 },
        { 0.5569, 1.9914E-04 }, { 0.6027, 1.4509E-04 }, { 0.6523, 1.0434E-04 }, { 0.7060, 7.3992E-05 },
        { 0.7640, 5.1700E-05 }, { 0.8268, 3.5558E-05 }, { 0.8948, 2.4046E-05 }, { 0.9684, 1.5971E-05 },
        { 1.048, 1.0404E-05 }, { 1.134, 6.6389E-06 }, { 1.228, 4.1430E-06 }, { 1.328, 2.5242E-06 },
        { 1.438, 1.4987E-06 }, { 1.556, 8.6532E-07 }, { 1.684, 4.8476E-07 }, { 1.822, 2.6281E-07 },
        { 1.972, 1.3749E-07 }, { 2.134, 6.9188E-08 }, { 2.310, 3.3371E-08 }, { 2.500, 1.5364E-08 }
    };

    for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
        double rate = test_data[i].rate;
        BOOST_CHECK_CLOSE( rate, im_rate_rel.lambda(test_data[i].im), 0.5);

        /*
         * Do a quick sanity check on the derivative function:
         */
        {
            double deriv = im_rate_rel.DerivativeAt(test_data[i].im);
            double epsilon = 1E-4;
            BOOST_CHECK_CLOSE( rate + deriv * epsilon, 
                               im_rate_rel.lambda(test_data[i].im + epsilon),
                               0.5);
        }
    }

    {
        shared_ptr<DeterministicFn> double_im_rate_function(
            new NonLinearHyperbolicLaw(2 * 1221, 29.8, 62.2));
        im_rate_function->replace(double_im_rate_function);
        im_rate_function = double_im_rate_function;
    }
    for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
        double rate = 2.0 * test_data[i].rate;
        BOOST_CHECK_CLOSE( rate, im_rate_rel.lambda(test_data[i].im), 0.5);

        /*
         * Do a quick sanity check on the derivative function:
         */
        {
            double deriv = im_rate_rel.DerivativeAt(test_data[i].im);
            double epsilon = 1E-4;
            BOOST_CHECK_CLOSE( rate + deriv * epsilon, 
                               im_rate_rel.lambda(test_data[i].im + epsilon),
                               0.5);
        }
    }
}

BOOST_AUTO_TEST_CASE( collapse )
{
    Integration::IntegrationSettings::Reset();
    double x[25] = {0.01, 0.02, 0.04, 0.06, 0.08, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6,
                    0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8,
                    1.9, 2.0};
    double y[25] = {0.376775, 0.155158, 0.054048, 0.027886, 0.017241, 0.011857,
                    0.003656, 0.001628, 0.000794, 0.000405, 0.000214, 0.000115,
                    6.4e-05, 3.7e-05, 2.1e-05, 1.3e-05, 7e-06, 4e-06, 3e-06,
                    1.8e-06, 1e-06, 7e-07, 4e-07, 2e-07, 1e-07};


    std::shared_ptr<LogLogInterpolatedFn> fn = std::make_shared<LogLogInterpolatedFn>(x, y, 25);

    struct { double im; double lambda; } test_data[] = {
        {0.01, 0.376775}, {0.0102828, 0.363564}, {0.0105736, 0.350816}, {0.0108726, 0.338515},
        {0.01118, 0.326645}, {0.0114962, 0.315192}, {0.0118213, 0.30414}, {0.0121556, 0.293476},
        {0.0124993, 0.283185}, {0.0128528, 0.273256}, {0.0132162, 0.263674}, {0.01359, 0.254429},
        {0.0139743, 0.245508}, {0.0143695, 0.236899}, {0.0147758, 0.228593}, 
        {0.0151937, 0.220577}, {0.0156233, 0.212843}, {0.0160651, 0.20538}, 
        {0.0165194, 0.198179}, {0.0169866, 0.19123}, {0.0174669, 0.184524}, 
        {0.0179609, 0.178054}, {0.0184688, 0.171811}, {0.018991, 0.165787}, {0.0195281, 0.159974},
        {0.0200803, 0.154215}, {0.0206481, 0.147809}, {0.021232, 0.141669}, {0.0218325, 0.135784},
        {0.0224498, 0.130144}, {0.0230847, 0.124738}, {0.0237375, 0.119556}, {0.0244088, 0.11459},
        {0.025099, 0.10983}, {0.0258088, 0.105268}, {0.0265386, 0.100895}, {0.0272891, 0.0967039},
        {0.0280608, 0.0926869}, {0.0288543, 0.0888368}, {0.0296703, 0.0851465}, 
        {0.0305093, 0.0816096}, {0.0313721, 0.0782196}, {0.0322592, 0.0749704}, 
        {0.0331715, 0.0718562}, {0.0341095, 0.0688713}, {0.0350741, 0.0660105}, 
        {0.0360659, 0.0632684}, {0.0370858, 0.0606403}, {0.0381346, 0.0581214}, 
        {0.039213, 0.055707}, {0.0403218, 0.0533457}, {0.0414621, 0.0509722}, 
        {0.0426346, 0.0487044}, {0.0438402, 0.0465374}, {0.04508, 0.0444669}, 
        {0.0463548, 0.0424884}, {0.0476656, 0.040598}, {0.0490135, 0.0387918},
        {0.0503996, 0.0370658}, {0.0518248, 0.0354167}, {0.0532903, 0.0338409},
        {0.0547973, 0.0323353}, {0.0563469, 0.0308966}, {0.0579403, 0.029522}, 
        {0.0595788, 0.0282085}, {0.0612636, 0.0269313}, {0.0629961, 0.0257049},
        {0.0647775, 0.0245343}, {0.0666093, 0.023417}, {0.0684929, 0.0223506}, 
        {0.0704298, 0.0213328}, {0.0724215, 0.0203613}, {0.0744695, 0.019434}, 
        {0.0765754, 0.018549}, {0.0787408, 0.0177043}, {0.0809675, 0.0168968}, 
        {0.0832571, 0.0161245}, {0.0856115, 0.0153875}, {0.0880325, 0.0146841},
        {0.0905219, 0.014013}, {0.0930818, 0.0133725}, {0.095714, 0.0127612}, 
        {0.0984207, 0.0121779}, {0.101204, 0.0116186}, {0.104066, 0.0110814}, 
        {0.107009, 0.0105691}, {0.110035, 0.0100805}, {0.113146, 0.00961448}, 
        {0.116346, 0.00916999}, {0.119636, 0.00874605}, {0.123019, 0.00834171},
        {0.126498, 0.00795606}, {0.130075, 0.00758824}, {0.133754, 0.00723743},
        {0.137536, 0.00690283}, {0.141425, 0.00658371}, {0.145425, 0.00627933},
        {0.149537, 0.00598903}, {0.153766, 0.00571215}, {0.158114, 0.00544807},
        {0.162585, 0.0051962}, {0.167183, 0.00495598}, {0.171911, 0.00472685}, 
        {0.176772, 0.00450833}, {0.181771, 0.0042999}, {0.186911, 0.00410111}, 
        {0.192197, 0.00391151}, {0.197632, 0.00373068}, {0.20322, 0.00354131}, 
        {0.208967, 0.00334965}, {0.214876, 0.00316837}, {0.220953, 0.00299689},
        {0.227201, 0.0028347}, {0.233626, 0.00268128}, {0.240233, 0.00253616}, 
        {0.247026, 0.0023989}, {0.254012, 0.00226907}, {0.261195, 0.00214627}, 
        {0.268581, 0.00203011}, {0.276176, 0.00192024}, {0.283986, 0.00181631},
        {0.292017, 0.00171801}, {0.300275, 0.00162429}, {0.308766, 0.00151508},
        {0.317497, 0.00141321}, {0.326476, 0.0013182}, {0.335708, 0.00122957}, 
        {0.345201, 0.0011469}, {0.354963, 0.00106979}, {0.365001, 0.000997865},
        {0.375323, 0.000930774}, {0.385937, 0.000868194}, {0.39685, 0.000809822},
        {0.408073, 0.000747552}, {0.419612, 0.000687234}, {0.431478, 0.000631783},
        {0.44368, 0.000580806}, {0.456227, 0.000533942}, {0.469128, 0.00049086}, 
        {0.482394, 0.000451254}, {0.496036, 0.000414843}, {0.510063, 0.000377726},
        {0.524487, 0.000342612}, {0.539319, 0.000310763}, {0.55457, 0.000281874}, 
        {0.570252, 0.000255671}, {0.586378, 0.000231904}, {0.60296, 0.000209798}, 
        {0.620011, 0.000187504}, {0.637544, 0.000167578}, {0.655573, 0.00014977},
        {0.674112, 0.000133855}, {0.693175, 0.00011963}, {0.712777, 0.000106224},
        {0.732933, 9.39871e-05}, {0.75366, 8.31603e-05}, {0.774972, 7.35806e-05}, 
        {0.796887, 6.51045e-05}, {0.819422, 5.72418e-05}, {0.842594, 5.02771e-05},
        {0.866422, 4.41597e-05}, {0.890923, 3.87867e-05}, {0.916117, 3.36328e-05},
        {0.942023, 2.89506e-05}, {0.968663, 2.49203e-05}, {0.996055, 2.1451e-05}, 
        {1.02422, 1.86174e-05}, {1.05319, 1.61801e-05}, {1.08297, 1.40619e-05}, 
        {1.11359, 1.19123e-05}, {1.14508, 9.76863e-06}, {1.17747, 8.01072e-06}, 
        {1.21076, 6.57638e-06}, {1.245, 5.41146e-06}, {1.28021, 4.45289e-06},
        {1.31641, 3.80988e-06}, {1.35364, 3.41899e-06}, {1.39192, 3.0682e-06},
        {1.43128, 2.54726e-06}, {1.47175, 2.07207e-06}, {1.51337, 1.66024e-06},
        {1.55617, 1.28787e-06}, {1.60017, 9.99362e-07}, {1.64542, 8.48147e-07}, 
        {1.69195, 7.19812e-07}, {1.7398, 5.5808e-07}, {1.789, 4.24741e-07}, 
        {1.83959, 3.02641e-07}, {1.89161, 2.11673e-07}, {1.9451, 1.4566e-07}, 
        {2.00011, 1e-07}, {2.05667, 1e-07}, {2.11483, 1e-07}, {2.17463, 1e-07},
        {2.23613, 1e-07}, {2.29936, 1e-07}, {2.36439, 1e-07}, {2.43125, 1e-07},
        {2.5, 1e-07}};
    
    for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
        BOOST_CHECK_CLOSE( test_data[i].lambda,
                           fn->ValueAt(test_data[i].im),
                           0.1);
    }

    IM im_rate_rel(fn);

    for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
        BOOST_CHECK_CLOSE( test_data[i].lambda,
                           im_rate_rel.lambda(test_data[i].im),
                           0.1);
        BOOST_CHECK_EQUAL( 0.0,
                           im_rate_rel.pCollapse(test_data[i].im));
    }
    BOOST_CHECK_EQUAL( 0.0, im_rate_rel.CollapseRate());

    im_rate_rel.SetCollapse(LogNormalDist::LogNormalDist_from_mean_X_and_sigma_lnX(0.9, 0.470));
    for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
        BOOST_CHECK_CLOSE( test_data[i].lambda,
                           im_rate_rel.lambda(test_data[i].im),
                           0.1);
    }
    
    struct {double im; double pCollapse; } test_collapse[] = {
        {0.01, 4.85951e-21}, {0.0225758, 1.40779e-14}, {0.0351515, 1.32857e-11},
        {0.0477273, 9.06647e-10}, {0.060303, 1.73312e-08}, {0.0728788, 1.58475e-07},
        {0.0854545, 9.01294e-07}, {0.0980303, 3.69262e-06}, {0.110606, 1.19222e-05},
        {0.123182, 3.21637e-05}, {0.135758, 7.54696e-05}, {0.148333, 0.000158481},
        {0.160909, 0.000304153}, {0.173485, 0.000541982}, {0.186061, 0.000907708},
        {0.198636, 0.00144255}, {0.211212, 0.00219208}, {0.223788, 0.00320481}, 
        {0.236364, 0.00453068}, {0.248939, 0.00621954}, {0.261515, 0.00831971}, 
        {0.274091, 0.0108766}, {0.286667, 0.0139318}, {0.299242, 0.0175219}, 
        {0.311818, 0.021678}, {0.324394, 0.0264252}, {0.33697, 0.0317827},
        {0.349545, 0.0377629}, {0.362121, 0.0443725}, {0.374697, 0.0516121},
        {0.387273, 0.0594766}, {0.399848, 0.0679559}, {0.412424, 0.0770349},
        {0.425, 0.0866945}, {0.437576, 0.0969118}, {0.450152, 0.107661}, 
        {0.462727, 0.118913}, {0.475303, 0.130636}, {0.487879, 0.1428},
        {0.500455, 0.155369}, {0.51303, 0.168308}, {0.525606, 0.181584}, 
        {0.538182, 0.19516}, {0.550758, 0.209001}, {0.563333, 0.223072}, 
        {0.575909, 0.237341}, {0.588485, 0.251772}, {0.601061, 0.266335},
        {0.613636, 0.280999}, {0.626212, 0.295733}, {0.638788, 0.310509},
        {0.651364, 0.3253}, {0.663939, 0.340081}, {0.676515, 0.354828},
        {0.689091, 0.369518}, {0.701667, 0.38413}, {0.714242, 0.398645}, 
        {0.726818, 0.413045}, {0.739394, 0.427312}, {0.75197, 0.441433}, 
        {0.764545, 0.455392}, {0.777121, 0.469178}, {0.789697, 0.482778}, 
        {0.802273, 0.496184}, {0.814848, 0.509385}, {0.827424, 0.522374}, 
        {0.84, 0.535144}, {0.852576, 0.547689}, {0.865152, 0.560004}, 
        {0.877727, 0.572084}, {0.890303, 0.583927}, {0.902879, 0.59553}, 
        {0.915455, 0.606891}, {0.92803, 0.618008}, {0.940606, 0.628882}, 
        {0.953182, 0.639511}, {0.965758, 0.649896}, {0.978333, 0.660037}, 
        {0.990909, 0.669937}, {1.00348, 0.679597}, {1.01606, 0.689019}, 
        {1.02864, 0.698205}, {1.04121, 0.707157}, {1.05379, 0.715879}, 
        {1.06636, 0.724373}, {1.07894, 0.732644}, {1.09152, 0.740693}, 
        {1.10409, 0.748526}, {1.11667, 0.756145}, {1.12924, 0.763554}, 
        {1.14182, 0.770758}, {1.15439, 0.77776}, {1.16697, 0.784565}, 
        {1.17955, 0.791176}, {1.19212, 0.797598}, {1.2047, 0.803835}, 
        {1.21727, 0.809892}, {1.22985, 0.815771}, {1.24242, 0.821478},
        {1.255, 0.827016}, {1.26758, 0.83239}, {1.28015, 0.837604}, 
        {1.29273, 0.842662}, {1.3053, 0.847567}, {1.31788, 0.852325}, 
        {1.33045, 0.856938}, {1.34303, 0.861411}, {1.35561, 0.865747},
        {1.36818, 0.86995}, {1.38076, 0.874024}, {1.39333, 0.877973}, 
        {1.40591, 0.8818}, {1.41848, 0.885508}, {1.43106, 0.8891},
        {1.44364, 0.892581}, {1.45621, 0.895954}, {1.46879, 0.899221},
        {1.48136, 0.902385}, {1.49394, 0.90545}, {1.50652, 0.908419}, 
        {1.51909, 0.911295}, {1.53167, 0.91408}, {1.54424, 0.916776}, 
        {1.55682, 0.919388}, {1.56939, 0.921917}, {1.58197, 0.924366},
        {1.59455, 0.926737}, {1.60712, 0.929033}, {1.6197, 0.931256}, 
        {1.63227, 0.933409}, {1.64485, 0.935492}, {1.65742, 0.93751}, 
        {1.67, 0.939463}, {1.68258, 0.941355}, {1.69515, 0.943186}, 
        {1.70773, 0.944958}, {1.7203, 0.946674}, {1.73288, 0.948336}, 
        {1.74545, 0.949944}, {1.75803, 0.951501}, {1.77061, 0.953008},
        {1.78318, 0.954468}, {1.79576, 0.95588}, {1.80833, 0.957248}, 
        {1.82091, 0.958572}, {1.83348, 0.959854}, {1.84606, 0.961095},
        {1.85864, 0.962297}, {1.87121, 0.96346}, {1.88379, 0.964587}, 
        {1.89636, 0.965677}, {1.90894, 0.966733}, {1.92152, 0.967755},
        {1.93409, 0.968745}, {1.94667, 0.969703}, {1.95924, 0.970631},
        {1.97182, 0.971529}, {1.98439, 0.972399}, {1.99697, 0.973242},
        {2.00955, 0.974057}, {2.02212, 0.974847}, {2.0347, 0.975612}, 
        {2.04727, 0.976353}, {2.05985, 0.97707}, {2.07242, 0.977765}, 
        {2.085, 0.978438}, {2.09758, 0.97909}, {2.11015, 0.979721}, 
        {2.12273, 0.980332}, {2.1353, 0.980925}, {2.14788, 0.981498}, 
        {2.16045, 0.982054}, {2.17303, 0.982592}, {2.18561, 0.983114},
        {2.19818, 0.983619}, {2.21076, 0.984108}, {2.22333, 0.984582},
        {2.23591, 0.985042}, {2.24848, 0.985487}, {2.26106, 0.985918},
        {2.27364, 0.986336}, {2.28621, 0.98674}, {2.29879, 0.987133}, 
        {2.31136, 0.987513}, {2.32394, 0.987881}, {2.33652, 0.988239},
        {2.34909, 0.988585}, {2.36167, 0.98892}, {2.37424, 0.989245}, 
        {2.38682, 0.98956}, {2.39939, 0.989866}, {2.41197, 0.990162}, 
        {2.42455, 0.990449}, {2.43712, 0.990727}, {2.4497, 0.990997}, 
        {2.46227, 0.991258}, {2.47485, 0.991512}, {2.48742, 0.991757},
        {2.5, 0.991996}};
    for (size_t i=0; 
         i < sizeof(test_collapse)/sizeof(test_collapse[0]);
         i++)
    {
        BOOST_CHECK_CLOSE( test_collapse[i].pCollapse,
                           im_rate_rel.pCollapse(test_collapse[i].im),
            1.0);
    }

    // {
    //     Integration::IntegrationSettings &settings = 
    //         im_rate_rel.Get_Integration_Settings();
    //     settings.Override_Tolerance(1E-6);
    //     settings.Override_Integration_Eval_Limit(5);
    // }

    BOOST_CHECK_CLOSE( 0.0002158956085998493,
                       im_rate_rel.CollapseRate(),
                       0.5);
}

BOOST_AUTO_TEST_CASE( Compound_Rate_Relationship_Test )
{
    shared_ptr<DeterministicFn> im_rate_function = std::make_shared<NonLinearHyperbolicLaw>(1221, 29.8, 62.2);

    shared_ptr<IM> im_rate_rel = std::make_shared<IM>(im_rate_function);
    
    shared_ptr<DeterministicFn> mu_edp = std::make_shared<PowerLawParametricCurve>(0.1, 1.5);

    shared_ptr<DeterministicFn> sigma_edp = std::make_shared<PowerLawParametricCurve>(0.5, 0.0);

    shared_ptr<ProbabilisticFn> edp_im_relationship = 
        std::make_shared<LogNormalFn>(mu_edp, LogNormalFn::MEAN_X, sigma_edp, LogNormalFn::SIGMA_LN_X);

    EDP rel(im_rate_rel, edp_im_relationship);

    const struct { double edp, rate; } test_data[] = {
        { 1.0000E-03, 7.8804E-02 }, { 1.0519E-03, 7.4990E-02 }, { 1.1065E-03, 7.1321E-02 }, { 1.1640E-03, 6.7801E-02 },
        { 1.2244E-03, 6.4421E-02 }, { 1.2880E-03, 6.1178E-02 }, { 1.3548E-03, 5.8068E-02 }, { 1.4252E-03, 5.5087E-02 },
        { 1.4992E-03, 5.2231E-02 }, { 1.5770E-03, 4.9495E-02 }, { 1.6589E-03, 4.6877E-02 }, { 1.7450E-03, 4.4373E-02 },
        { 1.8356E-03, 4.1978E-02 }, { 1.9309E-03, 3.9688E-02 }, { 2.0311E-03, 3.7502E-02 }, { 2.1365E-03, 3.5416E-02 },
        { 2.2475E-03, 3.3425E-02 }, { 2.3641E-03, 3.1527E-02 }, { 2.4869E-03, 2.9728E-02 }, { 2.6160E-03, 2.7994E-02 },
        { 2.7518E-03, 2.6354E-02 }, { 2.8946E-03, 2.4761E-02 }, { 3.0449E-03, 2.3311E-02 }, { 3.2030E-03, 2.1901E-02 },
        { 3.3693E-03, 2.0563E-02 }, { 3.5442E-03, 1.9293E-02 }, { 3.7282E-03, 1.8088E-02 }, { 3.9217E-03, 1.6947E-02 },
        { 4.1253E-03, 1.5866E-02 }, { 4.3395E-03, 1.4843E-02 }, { 4.5648E-03, 1.3875E-02 }, { 4.8018E-03, 1.2961E-02 },
        { 5.0511E-03, 1.2098E-02 }, { 5.3133E-03, 1.1283E-02 }, { 5.5891E-03, 1.0515E-02 }, { 5.8793E-03, 9.7908E-03 },
        { 6.1845E-03, 9.1079E-03 }, { 6.5056E-03, 8.4670E-03 }, { 6.8433E-03, 7.8642E-03 }, { 7.1986E-03, 7.2979E-03 },
        { 7.5723E-03, 6.7664E-03 }, { 7.9654E-03, 6.2680E-03 }, { 8.3790E-03, 5.8011E-03 }, { 8.8140E-03, 5.3639E-03 },
        { 9.2715E-03, 4.9550E-03 }, { 9.7529E-03, 4.5730E-03 }, { 1.0259E-02, 4.2163E-03 }, { 1.0792E-02, 3.8837E-03 },
        { 1.1352E-02, 3.5737E-03 }, { 1.1941E-02, 3.2809E-03 }, { 1.2561E-02, 3.0166E-03 }, { 1.3213E-02, 2.7671E-03 },
        { 1.3899E-02, 2.5356E-03 }, { 1.4621E-02, 2.3209E-03 }, { 1.5380E-02, 2.1220E-03 }, { 1.6179E-02, 1.9380E-03 },
        { 1.7018E-02, 1.7679E-03 }, { 1.7902E-02, 1.6109E-03 }, { 1.8831E-02, 1.4661E-03 }, { 1.9809E-02, 1.3326E-03 },
        { 2.0837E-02, 1.2099E-03 }, { 2.1919E-02, 1.0971E-03 }, { 2.3057E-02, 9.9351E-04 }, { 2.4254E-02, 8.9857E-04 },
        { 2.5513E-02, 8.1174E-04 }, { 2.6838E-02, 7.3222E-04 }, { 2.8231E-02, 6.5957E-04 }, { 2.9697E-02, 5.9334E-04 },
        { 3.1238E-02, 5.3301E-04 }, { 3.2860E-02, 4.7813E-04 }, { 3.4566E-02, 4.2828E-04 }, { 3.6361E-02, 3.8306E-04 },
        { 3.8248E-02, 3.4210E-04 }, { 4.0234E-02, 3.0505E-04 }, { 4.2323E-02, 2.7160E-04 }, { 4.4520E-02, 2.4143E-04 },
        { 4.6831E-02, 2.1426E-04 }, { 4.9262E-02, 1.8984E-04 }, { 5.1820E-02, 1.6792E-04 }, { 5.4510E-02, 1.4829E-04 },
        { 5.7340E-02, 1.3072E-04 }, { 6.0317E-02, 1.1504E-04 }, { 6.3448E-02, 1.0105E-04 }, { 6.6742E-02, 8.8606E-05 },
        { 7.0207E-02, 7.7550E-05 }, { 7.3852E-02, 6.7704E-05 }, { 7.7686E-02, 5.9070E-05 }, { 8.1719E-02, 5.1404E-05 },
        { 8.5961E-02, 4.4645E-05 }, { 9.0424E-02, 3.8696E-05 }, { 9.5118E-02, 3.3472E-05 }, { 0.1001, 2.8913E-05 },
        { 0.1053, 2.4887E-05 }, { 0.1107, 2.1391E-05 }, { 0.1165, 1.8346E-05 }, { 0.1225, 1.5699E-05 },
        { 0.1289, 1.3404E-05 }, { 0.1356, 1.1419E-05 }, { 0.1426, 9.7043E-06 }, { 0.1500, 8.2273E-06 },
    };
    for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
        BOOST_CHECK_CLOSE(rel.lambda(test_data[i].edp), test_data[i].rate, 0.2);
    }

    {
        std::shared_ptr<EDP> rel_ptr(
            new EDP(im_rate_rel, edp_im_relationship));
        
        for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
            BOOST_CHECK_CLOSE(rel_ptr->lambda(test_data[i].edp), test_data[i].rate, 0.2);
        }
        shared_ptr<DeterministicFn> double_im_rate_function = 
            std::make_shared<NonLinearHyperbolicLaw>(1 * 1221, 29.8, 62.2);
        im_rate_function->replace(double_im_rate_function);
        im_rate_function = double_im_rate_function;
    }

    {
        std::shared_ptr<EDP> rel_ptr = std::make_shared<EDP>(im_rate_rel, edp_im_relationship);

        for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
            BOOST_REQUIRE_CLOSE(rel_ptr->lambda(test_data[i].edp), test_data[i].rate, 0.2);
        }
    }

    {
        EDP *rel_ptr = new EDP(im_rate_rel, edp_im_relationship);

        for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
            BOOST_REQUIRE_CLOSE(rel_ptr->lambda(test_data[i].edp), test_data[i].rate, 0.2);
        }
        delete(rel_ptr);
    }

    {
        shared_ptr<DeterministicFn> double_im_rate_function = 
            std::make_shared<NonLinearHyperbolicLaw>(2 * 1221, 29.8, 62.2);
        im_rate_function->replace(double_im_rate_function);
        for (size_t i=0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
            BOOST_CHECK_CLOSE(rel.lambda(test_data[i].edp), 2.0 * test_data[i].rate, 0.2);
       }
    }
}


BOOST_AUTO_TEST_CASE( Callbacks )
{
    /*
     * Create a two simple relationships, using the same function. Destroy one
     * by letting it go out of scope, then replace the underlying function. This will cause a memory 
     * problem if the callbacks for the second function have not been cleaned up correctly.
     */
    shared_ptr<DeterministicFn> x_rate_fn = std::make_shared<NonLinearHyperbolicLaw>(1, 2, 3);
    shared_ptr<DeterministicFn> y_rate_fn = std::make_shared<NonLinearHyperbolicLaw>(1, 2, 3);
    shared_ptr<IM> x_im_rate = std::make_shared<IM>(x_rate_fn);
    {
        shared_ptr<IM> y_im_rate = std::make_shared<IM>(x_rate_fn);
    }
    x_rate_fn->replace(y_rate_fn);
    
    /*
     * Do pretty much the same but using compound relationships:
     */
    shared_ptr<DeterministicFn> x_mu_edp = std::make_shared<PowerLawParametricCurve>(0.1, 1.5);
    shared_ptr<DeterministicFn> x_sigma_edp = std::make_shared<PowerLawParametricCurve>(0.5, 0.0);
    shared_ptr<ProbabilisticFn> x_edp_im_fn = 
        std::make_shared<LogNormalFn>(x_mu_edp, LogNormalFn::MEAN_LN_X,  x_sigma_edp, LogNormalFn::SIGMA_LN_X);
    shared_ptr<EDP> x_edp_im_rel = std::make_shared<EDP>(x_im_rate, x_edp_im_fn); 
    {
        shared_ptr<EDP> y_edp_im_rel = std::make_shared<EDP>(x_im_rate, x_edp_im_fn); 
    }
    y_rate_fn->replace(x_rate_fn);
    x_mu_edp->replace(x_sigma_edp);
    x_sigma_edp->replace(x_mu_edp);
}


BOOST_AUTO_TEST_CASE( Compound_EDP_Test )
{
    shared_ptr<DeterministicFn> x_rate_fn = std::make_shared<NonLinearHyperbolicLaw>(1, 2, 3);
    shared_ptr<IM> x_im_rate = std::make_shared<IM>(x_rate_fn);
    
    shared_ptr<DeterministicFn> x_mu_edp_1 =
        std::make_shared<PowerLawParametricCurve>(0.1, 1.5);
    shared_ptr<DeterministicFn> x_sigma_edp_1 =
        std::make_shared<PowerLawParametricCurve>(0.5, 0.0);
    shared_ptr<ProbabilisticFn> x_edp_im_fn_1 = 
        std::make_shared<LogNormalFn>(x_mu_edp_1, LogNormalFn::MEAN_LN_X,
                                      x_sigma_edp_1, LogNormalFn::SIGMA_LN_X);
    shared_ptr<DeterministicFn> x_mu_edp_2 =
        std::make_shared<PowerLawParametricCurve>(0.2, 1.5);
    shared_ptr<DeterministicFn> x_sigma_edp_2 =
        std::make_shared<PowerLawParametricCurve>(0.5, 0.0);
    shared_ptr<ProbabilisticFn> x_edp_im_fn_2 = 
        std::make_shared<LogNormalFn>(x_mu_edp_2, LogNormalFn::MEAN_LN_X,
                                      x_sigma_edp_2, LogNormalFn::SIGMA_LN_X);
    shared_ptr<EDP> c_edp = std::make_shared<CompoundEDP>(x_im_rate, 
                                                           x_edp_im_fn_1,
                                                           x_edp_im_fn_2); 

    const int N=25;
    for (int i=0; i<N; i++) {
        double im = (double)i/N * 1.0;
        BOOST_CHECK_EQUAL(x_edp_im_fn_2->Mean(im), 
                          c_edp->Mean(im));
        BOOST_CHECK_EQUAL(x_edp_im_fn_2->MeanLn(im), 
                          c_edp->MeanLn(im));
        BOOST_CHECK_EQUAL(x_edp_im_fn_2->Median(im), 
                          c_edp->Median(im));
        BOOST_CHECK_EQUAL(x_edp_im_fn_2->SD(im), 
                          c_edp->SD(im));
        BOOST_CHECK_EQUAL(x_edp_im_fn_2->SD_ln(im), 
                          c_edp->SD_ln(im));
    }
    x_mu_edp_2 = std::make_shared<PowerLawParametricCurve>(0.2, 1.0);
    x_edp_im_fn_2 = std::make_shared<LogNormalFn>(x_mu_edp_2, LogNormalFn::MEAN_LN_X,
                                      x_sigma_edp_2, LogNormalFn::SIGMA_LN_X);
    c_edp = std::make_shared<CompoundEDP>(x_im_rate, 
                                          x_edp_im_fn_1,
                                          x_edp_im_fn_2); 
    for (int i=0; i<N; i++) {
        double im = (double)i/N * 5.0;
        if (x_edp_im_fn_1->Median(im) > x_edp_im_fn_2->Median(im)) {
            BOOST_CHECK_EQUAL(x_edp_im_fn_1->Mean(im), 
                              c_edp->Mean(im));
            BOOST_CHECK_EQUAL(x_edp_im_fn_1->MeanLn(im), 
                              c_edp->MeanLn(im));
            BOOST_CHECK_EQUAL(x_edp_im_fn_1->Median(im), 
                              c_edp->Median(im));
            BOOST_CHECK_EQUAL(x_edp_im_fn_1->SD(im), 
                              c_edp->SD(im));
            BOOST_CHECK_EQUAL(x_edp_im_fn_1->SD_ln(im), 
                              c_edp->SD_ln(im));
        } else {
            BOOST_CHECK_EQUAL(x_edp_im_fn_2->Mean(im), 
                              c_edp->Mean(im));
            BOOST_CHECK_EQUAL(x_edp_im_fn_2->MeanLn(im), 
                              c_edp->MeanLn(im));
            BOOST_CHECK_EQUAL(x_edp_im_fn_2->Median(im), 
                              c_edp->Median(im));
            BOOST_CHECK_EQUAL(x_edp_im_fn_2->SD(im), 
                              c_edp->SD(im));
            BOOST_CHECK_EQUAL(x_edp_im_fn_2->SD_ln(im), 
                              c_edp->SD_ln(im));
        }
    }
}
