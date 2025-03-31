#include "mzqc.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

using namespace mzqc;

int main() {
    std::ifstream csv_file("../CPTAC_CompRef_00_iTRAQ_01_2Feb12_Cougar_11-10-09_ids.csv");
    if (!csv_file.is_open()) {
        std::cerr << "Failed to open CSV file." << std::endl;
        return 1;
    }

    std::string line;
    std::getline(csv_file, line); // Skip header

    std::vector<std::shared_ptr<QualityMetric>> metrics;
    while (std::getline(csv_file, line)) {
        std::stringstream ss(line);
        std::string rt, peptide, target, mz, deltaPPM;

        std::getline(ss, rt, ',');
        std::getline(ss, peptide, ',');
        std::getline(ss, target, ',');
        std::getline(ss, mz, ',');
        std::getline(ss, deltaPPM, ',');

        nlohmann::json value = {
            {"RT", rt},
            {"peptide", peptide},
            {"MZ", mz},
            {"deltaPPM", deltaPPM}
        };

        // this is a example case as a POC for writing mzqc file, 
        //hence many simplifications  are made such as metric terms
        auto metric = std::make_shared<QualityMetric>(
            "QC:0000000", "Example Metric", "Example description", value, "unit"
        );
        metrics.push_back(metric);
    }

    csv_file.close();

    // Create input file
    auto inputFile = std::make_shared<InputFile>(
        "file:///path/to/input.mzML", 
        "input.mzML",
        std::make_shared<CvParameter>("MS:1000584", "mzML file", "", "PSI-MS")
    );
    
    // Create analysis software
    auto software = std::make_shared<AnalysisSoftware>(
        "MS:1000799", 
        "custom tool", 
        "1.0.0", 
        "http://example.org/tool"
    );

    // Create controlled vocabulary entries
    auto cv1 = std::make_shared<ControlledVocabulary>(
        "PSI-MS", 
        "https://github.com/HUPO-PSI/psi-ms-CV/blob/master/psi-ms.obo", 
        "4.1.55"
    );
    
    auto cv2 = std::make_shared<ControlledVocabulary>(
        "QC", 
        "https://github.com/HUPO-PSI/qcML-development/blob/master/cv/qc-cv.obo", 
        "0.1.0"
    );

    std::vector<std::shared_ptr<InputFile>> inputFiles = {inputFile};
    std::vector<std::shared_ptr<AnalysisSoftware>> software_list = {software};
    
    // Print debug info
    std::cout << "Created InputFile: " << inputFile->name << std::endl;
    std::cout << "Created AnalysisSoftware: " << software->name << std::endl;
    std::cout << "Created CV: " << cv1->name << " and " << cv2->name << std::endl;

    auto run_quality = std::make_shared<RunQuality>(
        "Example Run", 
        inputFiles,  // Use the vector we created
        software_list,  // Use the vector we created
        metrics
    );

    std::vector<std::shared_ptr<RunQuality>> run_qualities = {run_quality};
    std::vector<std::shared_ptr<SetQuality>> set_qualities;

    MzQCFile mzqc_file("", "1.0.0", "Contact Name", "Contact Address", "Description", run_qualities, set_qualities);
    
    // Add controlled vocabularies to the file
    mzqc_file.controlledVocabularies = {cv1, cv2};
    
    // Debug: Generate temp JSON to verify data
    nlohmann::json tempJson = mzqc_file.toJson();
    std::cout << "CV count: " << mzqc_file.controlledVocabularies.size() << std::endl;
    std::cout << "CV in JSON: " << (tempJson["mzQC"].contains("controlledVocabularies") ? "Yes" : "No") << std::endl;
    if (tempJson["mzQC"].contains("controlledVocabularies")) {
        std::cout << "CV JSON size: " << tempJson["mzQC"]["controlledVocabularies"].size() << std::endl;
    }
    
    // Debug: Verify inputFiles and analysisSoftware in runQualities
    std::cout << "RunQualities[0] has inputFiles: " << (run_quality->inputFiles.empty() ? "No" : "Yes") << std::endl;
    std::cout << "RunQualities[0] has analysisSoftware: " << (run_quality->analysisSoftware.empty() ? "No" : "Yes") << std::endl;

    std::ofstream output_file("output.mzqc");
    if (!output_file.is_open()) {
        std::cerr << "Failed to open output file." << std::endl;
        return 1;
    }

    // Use dump(4) to create the JSON with indentation for readability
    output_file << mzqc_file.toJson().dump(4);
    output_file.close();

    std::cout << "mzQC file generated successfully." << std::endl;
    return 0;
} 