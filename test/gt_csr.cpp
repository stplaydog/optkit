/**
 * Copy right YMSys, 2015, Zhaoming Yin
 *
 *  @brief    Unittest of CSR graph structure. 
 *
 *  MODIFIED   (MM/DD/YY)
 *  stplaydog   12/13/15 - Finished InitGraphTest_1 
 *  stplaydog   12/10/15 - Creation
**/

#include <gtest/gtest.h>
#include "csr.h"
#include "test_util.h" 

/**
 * @brief   Test a small graph one color
 *
**/
TEST(InitGraphTest_1, Success)
{
    /* Some basic setup */
    char file[OPTKIT_FILE_SIZE];
    snprintf(file, OPTKIT_FILE_SIZE, "%s", 
            "../data/MC/janc.gr");
    CSR g(file);

    /* Test some basic graph properties */
    ASSERT_EQ(g.get_num_v(), 8);
    ASSERT_EQ(g.get_num_e(), 26);
    ASSERT_EQ(g.get_num_c(), 1);

    /* Test the content of the graph */
    g.visualize();
    ASSERT_EQ(TstUtil::compareFile("../QA/unittest/MC/csr.dot", "./csr.dot"), 
            TstUtil::OPTKIT_TEST_PASS); 
}

/**
 * @brief   test compute_num_edge_intersect
 *
**/
TEST(ComputeNumEdgeIntersectTest_1, Success)
{
    /* Some basic setup */
    char file[OPTKIT_FILE_SIZE];
    snprintf(file, OPTKIT_FILE_SIZE, "%s", 
            "../data/MC/janc.gr");
    CSR g(file);

    /* Test some basic graph properties */
    vector<pair<int32_t, int32_t> > v_rg;
    for(int i=0; i<g.get_num_v(); i++)
    {
        v_rg.push_back(g.get_e_range(i));
    }

    /* Compare the correctness of intersection */
    FILE *writer = fopen("./intersect.txt", "w"); 
    for(int i=0; i<g.get_num_v(); i++)
    {
        for(int j=i+1; j<g.get_num_v(); j++)
        {
            fprintf(writer, "%d %d %d\n", 
                    i, j, g.compute_num_edge_intersect(v_rg[i], v_rg[j]));
        }
    }
    fclose(writer);

    ASSERT_EQ(TstUtil::compareFile("../QA/unittest/MC/intersect.txt", 
                "./intersect.txt"), TstUtil::OPTKIT_TEST_PASS); 
}

/**
 * @brief   test reconstruct  
 *
**/
TEST(ReconstructTest_1, Success)
{
}

/**
 * @brief   test output_all_CC 
 *
**/
TEST(OutputAllCCTest_1, Success)
{
}
