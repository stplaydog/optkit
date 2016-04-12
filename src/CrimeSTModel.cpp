/**
 * Copy right YMSys, 2015,2016 Zhaoming Yin
 *
 * @brief    This is the class for CSR formatted graph. 
 *
 *  MODIFIED   (MM/DD/YY)
 *  stplaydog   03/27/16 - add union find related functions 
 *  stplaydog   03/03/16 - add function to intepret CC files
 *  stplaydog   02/12/16 - add build_edges 
 *  stplaydog   02/11/16 - add query_list     
 *  stplaydog   02/10/16 - CSV read and RTree creation/query 
 *  stplaydog   02/09/16 - Creation
 *
**/

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include "../libs/CSVparser.h" 
#include "CrimeSTModel.h" 
#include "utils.h" 

/**
 * @brief       build graph from ny crime data
**/
void CrimeSTModel::read_data()
{
    csv::Parser file = csv::Parser(in_file.c_str());
    struct tm tm;

    for(uint32_t i=0; i<file.rowCount(); i++)
    {
        Node n;

        n.id = serial_num++;
        strptime(file[i][0].c_str(),"%Y/%m/%d", &tm); 
        n.coord[2] = tm.tm_mon * 30 + tm.tm_mday;
        n.coord[0] = stoi(file[i][1]);
        n.coord[1] = stoi(file[i][2]);
        n.freq = stoi(file[i][3]);
        n.type = file[i][4];

        nodes.push_back(n);
    }
}

/**
 * @brief       build graph from ny crime data
**/
void CrimeSTModel::build_model()
{
    for(uint32_t i=0; i<nodes.size(); i++)
    {
        rt.Insert(nodes[i].coord, nodes[i].coord, &(nodes[i]));
    } 
}

/**
 * @brief       given a range, return node ids in that range
 *
 * @param[in]       min         the min range
 * @param[in]       max         the max range
 *
 * @reurn       list of ids
**/
vector<int32_t> CrimeSTModel::query_list(int32_t min[3], int32_t max[3])
{
    ASSERT(min && max);

    vector<int32_t> ret;
    rt.Search(min, max, list_callback, &ret);
    return ret;
}

/**
 * @brief       given a range, return # nodes 
 *
 * @param[in]       min         the min range
 * @param[in]       max         the max range
 *
 * @reurn      # of nodes 
**/
int32_t CrimeSTModel::query_cont(int32_t min[3], int32_t max[3])
{
    ASSERT(min && max);

    int32_t nhits = rt.Search(min, max, cont_callback, NULL);
    return nhits;
}

/**
 * @brief       given model biult and the specified range, build edges based on the range relationship
 *              for example, given a vertex and a range, each vertex in that range will have an egde
 *              between this vertex.
 *
 * @param[in]       x_gap       range in x coordinate
 * @param[in]       y_gap       range in y coordinate
 * @param[in]       z_gap       range in z coordinate
 *
 * @return      a list of egdes
**/
edge_list CrimeSTModel::build_edges(int32_t x_gap, int32_t y_gap, int32_t z_gap)
{
    ASSERT(x_gap>=0 && y_gap>=0 && z_gap>=0);

    int32_t min[3];
    int32_t max[3];
    vector<pair<int32_t, int32_t>> ret;

    for(auto n : nodes)
    {
        min[0] = n.coord[0] - x_gap; min[1] = n.coord[1] - y_gap; min[2] = n.coord[2] - z_gap;
        max[0] = n.coord[0] + x_gap; max[1] = n.coord[1] + y_gap; max[2] = n.coord[2] + z_gap;

        vector<int32_t> v = query_list(min, max);

        for(auto i = v.begin(); i != v.end(); ++i)
        {
            if(*i > n.id)
            {
                ret.push_back(pair<int32_t, int32_t> (n.id, *i));
                ret.push_back(pair<int32_t, int32_t> (*i, n.id));
            }
        }
    }

    return ret;
}

/**
 * @brief       get the edge list and group them by connected components
 *
 * @param[in]       x_gap       range in x coordinate
 * @param[in]       y_gap       range in y coordinate
 * @param[in]       z_gap       range in z coordinate
 *
 * @return          edge list of connected components 
**/
edge_list_CC CrimeSTModel::build_edge_list_CC(int32_t x_gap, int32_t y_gap, int32_t z_gap)
{
    edge_list_CC ret;

    edge_list el = build_edges(x_gap, y_gap, z_gap);

    vector<int32_t> parent(nodes.size(), -1);
    union_find(el, parent);

    int32_t cur_CC = -1;
    int32_t cnt = 0;
    std::map<int32_t, int32_t> dic;

    for(auto it = el.begin(); it != el.end(); ++it)
    {
        int CC_id = find(parent, it->first);

        if(CC_id != cur_CC)
        {
            edge_list now_el;
            ret.push_back(now_el);
            cur_CC = CC_id;
            cnt = 0;
            dic.clear();
        }
        if(dic.find(it->first) == dic.end())
            dic[it->first] = cnt++;
        if(dic.find(it->second) == dic.end())
            dic[it->second] = cnt++;
        int32_t from = dic[it->first];
        int32_t to   = dic[it->second];
        ret.back().push_back(pair<int32_t, int32_t>(from, to));
    }

    return ret;
}

/**
 * @brief       union find algorithm to group connected components
 *
 * @param[in]       edge_list       list of edges
 * @param[out]      parent          parent index list
 *
 * @return      N/A
**/
void CrimeSTModel::union_find(const edge_list &el, vector<int32_t> &parent)
{
    for(auto it = el.begin(); it != el.end(); ++it)
    {
        int32_t from = find(parent, it->first);
        int32_t to   = find(parent, it->second);
        if(from != to)
        {
            parent[from] = to;
        }
    }
}

/**
 * @brief       find the parent in the union find algorithm
 *
 * @param[in]       parent          parent index list
 * @param[in]       pos             which position to search
 *
 * @return      the root parent idx
**/
int32_t CrimeSTModel::find(const vector<int32_t> &parent, int32_t pos)
{
    while(parent[pos] != -1)
    {
        pos = parent[pos];
    }
    return pos;
}

/**
 * @brief       serialize model into file, for test purpose
**/
void CrimeSTModel::serialize()
{
    FILE *writer = fopen("./crime_data.txt", "w");
    fprintf(writer, "%s %d\n", in_file.c_str(), serial_num);
    for(uint32_t i=0; i<nodes.size(); i++)
    {
        fprintf(writer, "%d %d|%d|%d %d %s\n", nodes[i].id, 
                nodes[i].coord[0], nodes[i].coord[1], nodes[i].coord[2],
                nodes[i].freq, nodes[i].type.c_str());
    }
    fclose(writer);
}

/**
 * @brief       serialize edges into file, for test purpose
**/
void CrimeSTModel::serialize_edges(edge_list &edges)
{
    sort(edges.begin(), edges.end(), Utils::smaller);

    FILE *writer = fopen("crime_edges.txt", "w");
    for(auto e : edges)
    {
        fprintf(writer, "%d %d\n", e.first, e.second);
    }
    fclose(writer);
}

/**
 * @brief       Interpret CC files
 *
 * @param[in]       fIn         input file
 * @param[in]       fOut        output file
 *
 * @return      N/A
**/
void CrimeSTModel::interpret_CC(const char *fIn, const char *fOut)
{
    ifstream reader(fIn);
    ofstream writer(fOut);

    string line;
    while(getline(reader, line))
    {
        istringstream ss(line);
        string token;
        bool is_comp = false;
        while(getline(ss, token, ' '))
        {
            if(token == "Comp")
            {
                is_comp = true;
            }
            if(is_comp && isdigit(token[0]))
            {
                int id = stoi(token);
                writer<<"["
                      <<nodes[id].coord[0]<<", "
                      <<nodes[id].coord[1]<<", "
                      <<nodes[id].coord[2]<<"]";
            }
            else
            {
                writer<<token<<" ";
            }
        }
        writer<<endl;
    }

    reader.close();
    writer.close();
}
