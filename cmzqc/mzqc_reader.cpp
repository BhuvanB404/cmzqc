#include "mzqc.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <stdexcept>

// Function to display metric value in a readable format
void displayMetricValue(const nlohmann::json& value, int indent = 0) {
    std::string indentStr(indent, ' ');
    
    if (value.is_null()) {
        std::cout << "null";
    } else if (value.is_boolean()) {
        std::cout << (value.get<bool>() ? "true" : "false");
    } else if (value.is_number_integer()) {
        std::cout << value.get<int>();
    } else if (value.is_number_float()) {
        // Use fixed precision for floating point
        std::cout << std::fixed << std::setprecision(4) << value.get<double>();
    } else if (value.is_string()) {
        std::cout << "\"" << value.get<std::string>() << "\"";
    } else if (value.is_array()) {
        if (value.empty()) {
            std::cout << "[]";
        } else if (value.size() == 1) {
            // Single element, show on one line
            std::cout << "[ ";
            displayMetricValue(value[0], 0);
            std::cout << " ]";
        } else if (value.size() <= 10 && 
                  (value[0].is_number() || value[0].is_string() || value[0].is_boolean())) {
            // Small array of simple values, show on one line
            std::cout << "[ ";
            for (size_t i = 0; i < value.size(); ++i) {
                if (i > 0) std::cout << ", ";
                displayMetricValue(value[i], 0);
            }
            std::cout << " ]";
        } else {
            // Larger or complex array, show on multiple lines
            std::cout << "[" << std::endl;
            for (size_t i = 0; i < value.size(); ++i) {
                std::cout << indentStr << "  ";
                displayMetricValue(value[i], indent + 2);
                if (i < value.size() - 1) {
                    std::cout << ",";
                }
                std::cout << std::endl;
            }
            std::cout << indentStr << "]";
        }
    } else if (value.is_object()) {
        if (value.empty()) {
            std::cout << "{}";
        } else {
            std::cout << "{" << std::endl;
            size_t i = 0;
            for (auto it = value.begin(); it != value.end(); ++it, ++i) {
                std::cout << indentStr << "  \"" << it.key() << "\": ";
                displayMetricValue(it.value(), indent + 2);
                if (i < value.size() - 1) {
                    std::cout << ",";
                }
                std::cout << std::endl;
            }
            std::cout << indentStr << "}";
        }
    }
}

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <mzqc_file_path> [schema_file_path]" << std::endl;
    std::cerr << "  mzqc_file_path: Path to the mzQC file to read" << std::endl;
    std::cerr << "  schema_file_path: Optional path to the mzQC schema file for validation" << std::endl;
    std::cerr << "                    (defaults to 'mzqc_schema.json' in current directory)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string filePath = argv[1];
    std::string schemaPath = (argc > 2) ? argv[2] : "mzqc_schema.json";
    
    std::cout << "Reading mzQC file: " << filePath << std::endl;
    std::cout << "Using schema file: " << schemaPath << std::endl;
    
    try {
        // Load the mzQC file with schema validation
        auto mzqcFile = mzqc::MzQcFile::fromFile(filePath, schemaPath);
        
        // Display basic file info
        std::cout << "\n===== mzQC File Info =====" << std::endl;
        std::cout << "Version: " << mzqcFile->version << std::endl;
        std::cout << "Creation date: " << mzqcFile->creationDate << std::endl;
        
        // Count and show basic stats
        int totalRunQualities = mzqcFile->runQualities.size();
        int totalSetQualities = mzqcFile->setQualities.size();
        int totalRunMetrics = 0;
        int totalInputFiles = 0;
        int totalSetMetrics = 0;
        
        // Count metrics in run qualities
        for (const auto& run : mzqcFile->runQualities) {
            totalRunMetrics += run->metrics.size();
            totalInputFiles += run->inputFiles.size();
        }
        
        // Count metrics in set qualities
        for (const auto& set : mzqcFile->setQualities) {
            totalSetMetrics += set->metrics.size();
        }
        
        int totalMetrics = totalRunMetrics + totalSetMetrics;
        
        // Display counts
        std::cout << "\n===== File Contents =====" << std::endl;
        std::cout << "Run qualities: " << totalRunQualities << std::endl;
        std::cout << "Set qualities: " << totalSetQualities << std::endl;
        std::cout << "Input files: " << totalInputFiles << std::endl;
        std::cout << "Total quality metrics: " << totalMetrics << std::endl;
        
        // Show summary of metrics from run qualities
        if (totalRunQualities > 0) {
            std::cout << "\n===== Run Quality Metrics =====" << std::endl;
            
            for (size_t i = 0; i < mzqcFile->runQualities.size(); ++i) {
                const auto& run = mzqcFile->runQualities[i];
                std::cout << "Run " << (i+1) << " (" << run->label << "): " 
                          << run->metrics.size() << " metrics" << std::endl;
                
                // List metrics with basic info
                for (size_t j = 0; j < run->metrics.size(); ++j) {
                    const auto& metric = run->metrics[j];
                    std::cout << "  [" << (j+1) << "] " << metric->name;
                    if (!metric->accession.empty()) {
                        std::cout << " (" << metric->accession << ")";
                    }
                    if (!metric->unit.empty()) {
                        std::cout << " [" << metric->unit << "]";
                    }
                    std::cout << " = ";
                    displayMetricValue(metric->value);
                    std::cout << std::endl;
                }
            }
        }
        
        // Show summary of metrics from set qualities
        if (totalSetQualities > 0) {
            std::cout << "\n===== Set Quality Metrics =====" << std::endl;
            
            for (size_t i = 0; i < mzqcFile->setQualities.size(); ++i) {
                const auto& set = mzqcFile->setQualities[i];
                std::cout << "Set " << (i+1) << " (" << set->label << "): " 
                          << set->metrics.size() << " metrics" << std::endl;
                
                // List metrics with basic info
                for (size_t j = 0; j < set->metrics.size(); ++j) {
                    const auto& metric = set->metrics[j];
                    std::cout << "  [" << (j+1) << "] " << metric->name;
                    if (!metric->accession.empty()) {
                        std::cout << " (" << metric->accession << ")";
                    }
                    if (!metric->unit.empty()) {
                        std::cout << " [" << metric->unit << "]";
                    }
                    std::cout << " = ";
                    displayMetricValue(metric->value);
                    std::cout << std::endl;
                }
            }
        }
        
        std::cout << "\nSuccessfully parsed and validated mzQC file with " << totalMetrics << " quality metrics." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 