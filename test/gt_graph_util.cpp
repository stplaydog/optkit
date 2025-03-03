/**
 * Copy right YMSys, 2015,2016 Zhaoming Yin
 *
 *  @brief    Unittest of graph utilies.
 *
 *  MODIFIED   (MM/DD/YY)
 *  stplaydog   04/11/16 - make everything work
 *  stplaydog   03/27/16 - test NYC crime data 
 *  stplaydog   03/20/16 - When compiling boost::graphviz with gtest, 
 *                         there will be shared_ptr issues, we addressed this isue
 *                         by printing edges.
 *  stplaydog   03/16/16 - Add boost graph test code.
 *  stplaydog   08/10/15 - Add validation code Success.
 *  stplaydog   08/09/15 - Creation
**/

#include <gtest/gtest.h>
#include "bgl.h"
#include "STModel.h"
#include "CrimeSTModel.h" 
#include "test_util.h" 

using namespace boost;


/**
 * @brief   Test a small graph one color for janc graph
**/
TEST(BGLInitGraph_1, Success)
{
    BGL g("../data/MC/janc.gr");
    ofstream writer ("bgl_janc.dot");
    g.print_dependencies<Adj>(writer,  g.m_adj);
    g.print_dependencies<Adj1>(writer, g.m_adj1);
    g.print_udependencies<Udir>(writer, g.m_udir);
    writer.close();

    ASSERT_EQ(TstUtil::compareFile("../QA/unittest/bgl/bgl_janc.dot", "./bgl_janc.dot"),
              TstUtil::OPTKIT_TEST_PASS);

    std::remove("./bgl_janc.dot");
}

/**
 * @brief   Test a small graph one color for janc graph
**/
TEST(BGLGraphProperty_1, Success)
{
    BGL g("../data/MC/janc.gr");

    Stats::instance()->m_application = "stmodel";    
    Stats::instance()->add_one_CC(); 

    g.floyd_warshall();
    g.clustering_coeff();
    g.betweeness_centrality();
    g.all_cliques();

    ASSERT_EQ(Stats::instance()->get_content(Stats::DIAMETER), "3");
    ASSERT_EQ(Stats::instance()->get_content(Stats::CLUSTERCOEFF), "0.311508");
    ASSERT_EQ(Stats::instance()->get_content(Stats::BETWEENCENTRL), 
            "2.166667,3.500000,0.333333,4.833333,1.000000,0.666667,4.000000,1.500000");
    ASSERT_EQ(Stats::instance()->get_content(Stats::CLIQUE), 
            "2,3\n3,5");
    Stats::instance()->m_gProperty.clear(); 
}

/**
 * @brief   Test a small graph one color for janc graph
**/
TEST(BGLInitGraph_2, Success)
{
    Stats::instance()->add_one_CC(); 
    BGL g("../data/MC/jwang.gr");
    ofstream writer ("bgl_jwang.dot");
    g.print_dependencies<Adj>(writer,  g.m_adj);
    g.print_dependencies<Adj1>(writer, g.m_adj1);
    g.print_udependencies<Udir>(writer, g.m_udir);
    writer.close();

    ASSERT_EQ(TstUtil::compareFile("../QA/unittest/bgl/bgl_jwang.dot", "./bgl_jwang.dot"),
              TstUtil::OPTKIT_TEST_PASS);

    std::remove("./bgl_jwang.dot");
    Stats::instance()->m_gProperty.clear(); 
}

/**
 * @brief   Test a small graph one color for janc graph
**/
TEST(BGLGraphProperty_2, Success)
{
    BGL g("../data/MC/jwang.gr");

    Stats::instance()->m_application = "stmodel";

    Stats::instance()->add_one_CC(); 

    g.floyd_warshall();
    g.clustering_coeff();
    g.betweeness_centrality();
    g.all_cliques();

    ASSERT_EQ(Stats::instance()->get_content(Stats::DIAMETER), "3");
    ASSERT_EQ(Stats::instance()->get_content(Stats::CLUSTERCOEFF), "0.447373");
    ASSERT_EQ(Stats::instance()->get_content(Stats::BETWEENCENTRL), 
            "0.533333,0.000000,0.900000,9.433333,1.000000,0.000000,26.266667,0.900000,0.000000,9.233333,0.000000,0.000000,0.000000,0.000000,1.000000,0.333333,2.700000,97.300000,0.333333,2.033333,57.033333");
    ASSERT_EQ(Stats::instance()->get_content(Stats::CLIQUE), 
            "2,3\n3,12\n4,5");
    Stats::instance()->m_gProperty.clear(); 
}

/**
 * Analyze shootings in Philadelphia and find 
 * an elevated risk of near-repeat shootings 
 * occurring within 2 weeks and within one city 
 * block of previous incidents
 *
 * we only test 200 meter
**/
TEST(BGLSTModel_1, Success)
{
    string input_file = "../data/truss/ny_crime.csv"; 
    CrimeSTModel stm(input_file);

    edge_list_CC el_cc = stm.build_edge_list_CC(200, 200, 30);

    Stats::instance()->m_application = "stmodel";    
    Stats::instance()->m_outFile     = "./stats.txt";    
    Stats::instance()->serialize();

    for(auto it = el_cc.begin(); it != el_cc.end(); ++it)
    {
        BGL g(*it);
        Stats::instance()->add_one_CC(); 
        
        g.floyd_warshall();
        g.clustering_coeff();
        g.betweeness_centrality();
        g.all_cliques();
    }

    Stats::instance()->serialize();

    ASSERT_EQ(TstUtil::compareFile("../QA/unittest/bgl/nycrime_stats.txt", "./stats.txt"), 
            TstUtil::OPTKIT_TEST_PASS); 

    std::remove("./stats.txt");
    Stats::instance()->m_gProperty.clear(); 
}

TEST(BGLConnectedComponent_1, Success)
{
    string input_file = "../data/truss/ny_crime.csv"; 
    CrimeSTModel stm(input_file);

    edge_list el = stm.build_edges(200, 200, 30);

    vector<int32_t> parent(stm.nodes.size(), -1);
    stm.union_find(el, parent);
    vector<int32_t> comp(stm.nodes.size(), -1);
    set<int> st;
    for(size_t i=0; i<parent.size(); i++)
    {
        comp[i] = stm.find(parent, i); 
        st.insert(comp[i]);
    }

    BGL g(el);
    vector<int32_t> comp1(stm.nodes.size());

    map<int, int> mp;

    for(size_t i=0; i<comp.size(); i++)
    {
        auto it = mp.find(comp[i]);
        if(it == mp.end())
        {
            it->second = comp1[i];
        }
        else 
        {
            ASSERT_EQ(it->second, comp1[i]);
        }
    }
    Stats::instance()->m_gProperty.clear(); 
}
