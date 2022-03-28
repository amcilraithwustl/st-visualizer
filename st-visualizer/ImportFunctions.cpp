#include "ImportFunctions.h"
#include "UtilityFunctions.h"
#include <fstream>
#include <iostream>
#include <functional>
#include <iostream>
#include <algorithm>
#include <limits>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <utility>

#include "GrowAndCover.h"
using std::vector;
using std::string;

constexpr int wid_buffer = 2;
constexpr int num_ransac = 5;

//From https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<string> splitString(const string& s, const string& delimiter)
{
    size_t pos_start = 0, pos_end;
    const size_t delimiter_length = delimiter.length();
    std::vector<string> res;

    while((pos_end = s.find(delimiter, pos_start)) != string::npos)
    {
        string token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delimiter_length;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

std::vector<coord> extractCoordinateSet(std::vector<float> top, std::vector<float> bottom)
{
    if(top.size() != bottom.size()) throw std::exception("Size Mismatch");
    std::vector<coord> a;
    a.reserve(top.size());
    for(size_t i = 0; i < top.size(); i++)
    {
        a.emplace_back(coord(top[i], bottom[i]));
    }
    return a;
}

/// Any impossible fields default to inf
vector<float> convertRowToFloat(vector<string> input)
{
    return input << std::function([](string s)
        {
            try
            {
                return std::stof(s);
            }
            catch(...)
            {
                std::cerr << "WARNING: Invalid std::stoi attempt. Please check data: " << s << std::endl;
            }
            return std::numeric_limits<float>::infinity();
        }
    );
}

std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(const string& alignment_file)
{
    //Import raw file data
    std::ifstream aFile(alignment_file);
    std::vector<string> lines;
    if (aFile.is_open())
    {
        string line;
        while (std::getline(aFile, line))
        {
            lines.push_back(line);
        }
        aFile.close();
    }
    else { throw "ALIGNMENT FILE NOT FOUND"; };

    //Split into cells
    vector<vector<string>> csvCells;
    for(const string& row : lines)
    {
        vector<string> rowCells = splitString(row, ",");
        //Drop the first three columns
        csvCells.emplace_back(rowCells.begin() + 3, rowCells.end());
    }
    //Drop the first row
    csvCells = vector(csvCells.begin() + 1, csvCells.end());


    // Transform cells into ints
    const auto transformCells = csvCells << std::function(convertRowToFloat);

    //Now we have n columns and m*4 rows of ints
    //This part transforms that into an m by 2 by n structure of coords

    //Transpose pairs
    vector<vector<coord>> coordinateSet;
    for(size_t i = 0; i < transformCells.size(); i += 2)
    {
        coordinateSet.push_back(extractCoordinateSet(transformCells[i], transformCells[i + 1]));
    }

    vector<std::pair<vector<coord>, vector<coord>>> finalSet;
    //line up in pairs 
    for(size_t i = 0; i < coordinateSet.size(); i += 2)
    {
        finalSet.emplace_back(coordinateSet[i], coordinateSet[i + 1]);
    }

    return finalSet;
}

tsv_return_type loadTsv(const std::string& file_name, const std::vector<std::string>& slice_names,
                        unsigned int slice_index,
                        unsigned int tissue_index,
                        std::pair<
                            unsigned, unsigned> row_col_indices, unsigned int cluster_ind,
                        const std::vector<unsigned>& feature_indices, unsigned int z_distance,
                        std::vector<std::pair<std::vector<coord>, std::vector<coord>>> source_targets)
{
    //Import raw file data
    std::ifstream aFile(file_name);
    std::vector<string> lines;
    if(aFile.is_open())
    {
        string line;

        while(std::getline(aFile, line))
        {
            lines.push_back(line);
        }
        aFile.close();
    }
    std::vector<std::vector<std::string>> rawData = lines << std::function([](const string& line)
    {
        return splitString(line, "\t");
    });

    vector<string> names = feature_indices << std::function([rawData](unsigned index)
    {
        return rawData[0][index];
    });
    names.emplace_back("No Tissue");

    vector tab(rawData.begin() + 1, rawData.end());

    int max = 0;
    for(const auto& row : tab)
    {
        {
            try
            {
                int n = std::stoi(row[cluster_ind]);
                if(n > max)
                    max = n;
            }
            catch(...)
            {
            }
        }
    }

    unsigned int newClusters = static_cast<unsigned int>(max) + 1;
    auto newFeatures = feature_indices.size();

    //Extract the relevant records
    //Records should hold slices of data rows that match the right name of the slice and is a tissue sample
    vector<vector<vector<string>>> sliced_records = slice_names << std::function([&](const string& name)
    {
        return filter(tab, std::function([&](const vector<string>& row)
        {
            return row[slice_index] == name && row[tissue_index] == "1";
        }));
    });

    //This is the point where parallel vectors are created

    //Pull out the corresponding xy data from the records, adjusted to be a unified coordinate system
    std::pair xy_indices(row_col_indices.second, row_col_indices.first);
    vector<Eigen::Matrix2Xf> slices = sliced_records << std::function(
        [&](const vector<vector<string>>& record, size_t i)
        {
            const auto raw_slice_coordinates = record << std::function([&](const vector<string>& row)
            {
                return std::pair(std::stof(row[xy_indices.first]), std::stof(row[xy_indices.second]));
            });


            if(i == 0) //If it's the first slice, no adjustment necessary
            {
                return vectorToMatrix(raw_slice_coordinates);
            }

            //Base the remaining slices coordinate adjustment off of the previous one
            const std::function transform = getTransSVD(source_targets[i - 1].first, source_targets[i - 1].second);
            return vectorToMatrix(transform(raw_slice_coordinates));
        });

    //Convert the clusters into an array of 1/0 based on the cluster index.
    //Clusters are represented as vectors with all values zero, except a single 1 in the ith place where i is the cluster it belongs to
    vector<vector<vector<float>>> original_clusters = sliced_records << std::function(
        [&](const vector<vector<string>>& record)
        {
            return record << std::function([&](const vector<string>& row)
                {
                    return getClusterArray(
                        newClusters + 1,
                        row[tissue_index] == "0"
                        ? newClusters
                        : std::stoi(row[cluster_ind]
                            //If the data point doesn't contain tissue, give it a "none" value, otherwise use the cluster value it is a part of
                        ));
                }
            );
        });


    //Values are represented as arrays too, but the values are not just 1 or 0, but are all floats (except for the last index)
    vector<vector<vector<float>>> values = sliced_records << std::function([&](const vector<vector<string>>& record)
    {
        return record << std::function([&](const vector<string>& row)
        {
            if(row[tissue_index] == "0")
            {
                return getClusterArray(newFeatures + 1, newFeatures);
            }

            vector<float> a = feature_indices << std::function([row](unsigned index) { return std::stof(row[index]); });
            a.emplace_back(0);
            return a;
        });
    });


    //Add buffer to each slice and grow and cover neighboring slices
    vector<Eigen::Matrix2Xf> new_slice_data = slices << std::function([slices](const Eigen::Matrix2Xf&, size_t i)
    {
        if(i == 0)
        {
            return growAndCover(slices[i], slices[i + 1], wid_buffer, num_ransac);
        }

        if(i == slices.size() - 1)
        {
            return growAndCover(slices[i], slices[i - 1], wid_buffer, num_ransac);
        }


        Eigen::Matrix2Xf top_and_bottom_slice(2, slices[i + 1].cols() + slices[i - 1].cols());
        top_and_bottom_slice << slices[i + 1], slices[i - 1];
        return growAndCover(slices[i], top_and_bottom_slice, wid_buffer, num_ransac);
    });


    vector<Eigen::Matrix3Xf> slices3d = mapThread(
        new_slice_data, slices, std::function(
            [z_distance](const Eigen::Matrix2Xf& new_slice,
                         const Eigen::Matrix2Xf& old_slice, size_t i)
            {
                Eigen::Matrix2Xf layer2d(
                    2, new_slice.cols() + old_slice.cols());
                layer2d << old_slice, new_slice;
                //The old and new points on that layer (in that order)
                Eigen::Matrix3Xf layer3d(3, layer2d.cols());
                for(int j = 0; j < layer2d.cols(); j++)
                {
                    layer3d.col(j)(0) = layer2d.col(j)(0);
                    layer3d.col(j)(1) = layer2d.col(j)(1);
                    layer3d.col(j)(2) = static_cast<float>(z_distance * i);
                }

                return layer3d;
            }));


    //Associating data with the new points on their respective slices
    vector<vector<vector<float>>> grown_clusters = mapThread(
        new_slice_data, original_clusters, std::function(
            [&](const Eigen::Matrix2Xf& new_coordinates,
                const vector<vector<float>>& old_clusters)
            {
                //Associate null data with the areas that we grew to and append them to the list of locations
                const vector new_clusters(new_coordinates.cols(), getClusterArray(newClusters + 1, newClusters));
                return concat(old_clusters, new_clusters);
            }));


    vector<vector<vector<float>>> grown_values = mapThread(
        new_slice_data, values, std::function(
            [&](const Eigen::Matrix2Xf& new_coordinates,
                const vector<vector<float>>& old_clusters)
            {
                //Same process as the previous step, but this time for vals instead of clusters
                const vector tempVector(
                    new_coordinates.cols(),
                    getClusterArray(newFeatures + 1, newFeatures));
                return concat(old_clusters, tempVector);
            }));


    //Add empty top and bottom slices
    {
        auto& topSlice = slices3d[slices3d.size() - 1];
        Eigen::Matrix3Xf top(3, topSlice.cols());
        for(int i = 0; i < topSlice.cols(); i++)
        {
            top.col(i) = topSlice.col(i) + Eigen::Vector3f({0, 0, static_cast<float>(z_distance)});
        }
        slices3d.push_back(top);

        auto& bottomSlice = slices3d[0];
        Eigen::Matrix3Xf bottom(3, bottomSlice.cols());
        for(int i = 0; i < bottomSlice.cols(); i++)
        {
            bottom.col(i) = bottomSlice.col(i) - Eigen::Vector3f({0, 0, static_cast<float>(z_distance)});
        }
        slices3d.insert(slices3d.begin(), bottom);
        //TODO: Inefficient O(n) call. Consider changing in all three locations.
    }

    {
        auto& topSlice = grown_clusters[grown_clusters.size() - 1];
        vector top(topSlice.size(), getClusterArray(newClusters + 1, newClusters));
        grown_clusters.push_back(top);

        auto& bottomSlice = grown_clusters[0];
        vector bottom(bottomSlice.size(), getClusterArray(newClusters + 1, newClusters));
        grown_clusters.insert(grown_clusters.begin(), bottom);
    }

    {
        auto& topSlice = grown_values[grown_values.size() - 1];
        vector top(topSlice.size(), getClusterArray(newFeatures + 1, newFeatures));
        grown_values.push_back(top);

        auto& bottomSlice = grown_values[0];
        vector bottom(bottomSlice.size(), getClusterArray(newFeatures + 1, newFeatures));
        grown_values.insert(grown_values.begin(), bottom);
    }

    //TODO: std::move for speed reasons
    tsv_return_type ret;
    ret.names = names;
    ret.slices = slices3d;
    ret.values = grown_values;
    ret.clusters = grown_clusters;
    return ret;
}


Eigen::Vector2f getCentroid(colCoordMat sourceMatrix)
{
    return sourceMatrix.rowwise().mean();
}

colCoordMat translateToZeroCentroid(colCoordMat source_matrix)
{
    //Get the average of each row
    const auto centroid = getCentroid(source_matrix);

    //Subtract the centroid from each column;
    return source_matrix.colwise() - centroid;
}

Eigen::Matrix2f getSVDRotation(colCoordMat source_matrix, colCoordMat target_matrix)
{
    //Row 0 is x, row 1 is y

    //(* getting the centroid *)
    const colCoordMat zeroSource = translateToZeroCentroid(std::move(source_matrix));
    colCoordMat zeroTarget = translateToZeroCentroid(std::move(target_matrix));

    const Eigen::Matrix2f mat = zeroSource * zeroTarget.transpose();

    //(* SVD decomposition *)
    const Eigen::JacobiSVD<colCoordMat> svd(mat, Eigen::ComputeThinU | Eigen::ComputeThinV);
    //(* obtaining the rotation *)
    Eigen::Matrix2f r = (svd.matrixU() * svd.matrixV().transpose()).transpose(); //This is definitely a rotation matrix

    return r;
}

std::function<std::vector<coord>(std::vector<coord>)> getTransSVD(const std::vector<coord>& source,
                                                                  const std::vector<coord>& target)
{
    const auto sourceMatrix = vectorToMatrix(source);
    const auto targetMatrix = vectorToMatrix(target);

    //Convert to matrices
    const auto r = getSVDRotation(sourceMatrix, targetMatrix);
    const Eigen::Rotation2D<float> rotation(r);

    const Eigen::Vector2f sourceCentroid = getCentroid(sourceMatrix);
    const Eigen::Vector2f targetCentroid = getCentroid(targetMatrix);
    const Eigen::Vector2f targetCentroidTransform = targetCentroid - sourceCentroid;


    const Eigen::Translation2f netTranslation(targetCentroidTransform);
    const Eigen::Translation2f toZero(-1 * sourceCentroid);
    const Eigen::Translation2f fromZero(sourceCentroid);
    Eigen::Transform<float, 2, Eigen::Affine> finalTransform = netTranslation * fromZero * rotation * toZero;
    //Translate after rotate


    //(* transform *) Creating the function
    return {
        [finalTransform](std::vector<coord> points)
        {
            return matrixToVector(finalTransform * vectorToMatrix(std::move(points)));
        }
    };
}

std::vector<float> getClusterArray(size_t length, size_t i)
{
    std::vector<float> ret(length, 0);
    if(i < length)
    {
        ret[i] = 1;
    }
    return ret;
}
