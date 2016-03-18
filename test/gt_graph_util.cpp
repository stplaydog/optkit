/**
 * Copy right YMSys, 2015,2016 Zhaoming Yin
 *
 *  @brief    Unittest of graph utilies.
 *
 *  MODIFIED   (MM/DD/YY)
 *  stplaydog   03/16/16 - Add boost graph test code.
 *  stplaydog   08/10/15 - Add validation code Success.
 *  stplaydog   08/09/15 - Creation
**/

#include <gtest/gtest.h>
#include "bgl.h"
#include "test_util.h" 

using namespace boost;
#include <boost/graph/graphml.hpp>


/**
 * @brief   Test a small graph one color for janc graph
**/
TEST(BGLInitGraph_1, Success)
{
    BGL g("../data/MC/janc.gr");
    ofstream writer ("bgl_janc.dot");
//    write_graphviz(writer, g.m_adj);
//    write_graphviz(writer, g.m_adj1);
//    write_graphviz(writer, g.m_udir);
    writer.close();

//    ASSERT_EQ(TstUtil::compareFile("../QA/unittest/bgl/bgl_janc.dot", "./bgl_janc.dot"),
//              TstUtil::OPTKIT_TEST_PASS);

    std::remove("./bgl_janc.dot");
}

