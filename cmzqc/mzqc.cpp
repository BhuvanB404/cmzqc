#include "mzqc.hpp"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>

namespace mzqc {

// Static schema variable
static nlohmann::json schema = nullptr;

// Function to load and cache schema from file
nlohmann::json loadSchema(const std::string& schemaPath) {
    static bool schemaLoaded = false;
    static nlohmann::json cachedSchema;
    
    if (!schemaLoaded) {
        try {
            std::ifstream schemaFile(schemaPath);
            if (!schemaFile.is_open()) {
                throw std::runtime_error("Failed to open schema file: " + schemaPath);
            }
            
            schemaFile >> cachedSchema;
            schemaLoaded = true;
        } catch (const std::exception& e) {
            throw std::runtime_error("Error loading schema file: " + std::string(e.what()));
        }
    }
    
    return cachedSchema;
}

// Simple schema validation function - basic checks only, not full JSON Schema validation
bool validateAgainstSchema(const nlohmann::json& j, const std::string& schemaPath) {
    try {
        // Load the schema
        nlohmann::json schemaJson = loadSchema(schemaPath);
        
        // Basic validation - check if root mzQC object exists
        if (!j.contains("mzQC")) {
            std::cerr << "Schema validation error: missing root 'mzQC' object" << std::endl;
            return false;
        }
        
        const auto& mzqc = j["mzQC"];
        
        // Check required properties from schema
        if (!mzqc.contains("version") || !mzqc.contains("creationDate")) {
            std::cerr << "Schema validation error: missing required properties in mzQC object" << std::endl;
            return false;
        }
        
        // Check if at least runQualities or setQualities exists
        if (!mzqc.contains("runQualities") && !mzqc.contains("setQualities")) {
            std::cerr << "Schema validation error: either runQualities or setQualities must be present" << std::endl;
            return false;
        }
        
        // Check if controlledVocabularies exists
        if (!mzqc.contains("controlledVocabularies")) {
            std::cerr << "Schema validation error: controlledVocabularies must be present" << std::endl;
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Schema validation error: " << e.what() << std::endl;
        return false;
    }
}

// ControlledVocabulary implementation
ControlledVocabulary::ControlledVocabulary(const std::string& name,
                                           const std::string& uri,
                                           const std::string& version)
    : name(name), uri(uri), version(version) {}

nlohmann::json ControlledVocabulary::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["name"] = name;
    j["uri"] = uri;
    j["version"] = version;
    return j;
}

void ControlledVocabulary::fromJson(const nlohmann::json& j) {
    id = j.value("id", "");
    name = j.value("name", "");
    uri = j.value("uri", "");
    version = j.value("version", "");
}

// CvTermCache implementation
int CvTermCache::loadFromOboFile(const std::string& filename) {
    currentOboFile = filename;
    return parseOboFile(filename);
}

int CvTermCache::parseOboFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return -1;
    
    // Simplified implementation for OBO file parsing
    std::string line;
    CvTermDetails currentTerm;
    bool inTermDef = false;
    
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '!') continue;
        
        // Term start
        if (line == "[Term]") {
            if (inTermDef && !currentTerm.accession.empty()) {
                // Save previous term
                termCache[currentTerm.accession] = currentTerm;
            }
            // Reset for new term
            currentTerm = CvTermDetails();
            inTermDef = true;
            continue;
        }
        
        // Only process if we're inside a term definition
        if (!inTermDef) continue;
        
        // Process basic term data
        if (line.find("id:") == 0) {
            currentTerm.accession = line.substr(3);
        } else if (line.find("name:") == 0) {
            currentTerm.name = line.substr(5);
        } else if (line.find("def:") == 0) {
            currentTerm.definition = line.substr(4);
        } else if (line.find("is_a:") == 0) {
            currentTerm.parentTerms.push_back(line.substr(5));
        }
    }
    
    // Add the last term
    if (inTermDef && !currentTerm.accession.empty()) {
        termCache[currentTerm.accession] = currentTerm;
    }
    
    return termCache.size();
}

// CvParameter implementation, works
CvParameter::CvParameter(const std::string& accession,
                        const std::string& name,
                        const std::string& value,
                        const std::string& cvRef)
    : accession(accession), name(name), value(value), cvRef(cvRef) {}

nlohmann::json CvParameter::toJson() const {
    nlohmann::json j;
    j["accession"] = accession;
    j["name"] = name;
    if (!value.empty()) {
        j["value"] = value;
    }
    if (!cvRef.empty()) {
        j["cvRef"] = cvRef;
    }
    return j;
}

void CvParameter::fromJson(const nlohmann::json& j) {
    accession = j.value("accession", "");
    name = j.value("name", "");
    value = j.value("value", "");
    cvRef = j.value("cvRef", "");
}

// AnalysisSoftware implementation
AnalysisSoftware::AnalysisSoftware(const std::string& accession,
                                  const std::string& name,
                                  const std::string& version,
                                  const std::string& uri)
    : accession(accession), name(name), version(version), uri(uri) {}

nlohmann::json AnalysisSoftware::toJson() const {
    nlohmann::json j;
    j["accession"] = accession;
    j["name"] = name;
    j["version"] = version;
    if (!uri.empty()) {
        j["uri"] = uri;
    }
    return j;
}

void AnalysisSoftware::fromJson(const nlohmann::json& j) {
    accession = j.value("accession", "");
    name = j.value("name", "");
    version = j.value("version", "");
    uri = j.value("uri", "");
}

// InputFile implementation
InputFile::InputFile(const std::string& location,
                    const std::string& name,
                    const std::shared_ptr<CvParameter>& fileFormat,
                    const std::vector<std::shared_ptr<CvParameter>>& fileProperties)
    : location(location), name(name), fileFormat(fileFormat), fileProperties(fileProperties) {}

nlohmann::json InputFile::toJson() const {
    nlohmann::json j;
    j["location"] = location;
    j["name"] = name;
    if (fileFormat) {
        j["fileFormat"] = fileFormat->toJson();
    }
    if (!fileProperties.empty()) {
        j["fileProperties"] = nlohmann::json::array();
        for (const auto& prop : fileProperties) {
            j["fileProperties"].push_back(prop->toJson());
        }
    }
    return j;
}

void InputFile::fromJson(const nlohmann::json& j) {
    location = j.value("location", "");
    name = j.value("name", "");
    
    if (j.contains("fileFormat")) {
        fileFormat = std::make_shared<CvParameter>();
        fileFormat->fromJson(j["fileFormat"]);
    }
    
    if (j.contains("fileProperties") && j["fileProperties"].is_array()) {
        fileProperties.clear();
        for (const auto& prop : j["fileProperties"]) {
            auto cvParam = std::make_shared<CvParameter>();
            cvParam->fromJson(prop);
            fileProperties.push_back(cvParam);
        }
    }
}

// QualityMetric implementation
QualityMetric::QualityMetric(const std::string& accession,
                            const std::string& name,
                            const std::string& description,
                            const nlohmann::json& value,
                            const std::string& unit)
    : accession(accession), name(name), description(description), value(value), unit(unit) {}

nlohmann::json QualityMetric::toJson() const {
    nlohmann::json j;
    j["accession"] = accession;
    j["name"] = name;
    if (!description.empty()) {
        j["description"] = description;
    }
    if (!value.is_null()) {
        j["value"] = value;
    }
    if (!unit.empty()) {
        j["unit"] = unit;
    }
    return j;
}

void QualityMetric::fromJson(const nlohmann::json& j) {
    accession = j.value("accession", "");
    name = j.value("name", "");
    description = j.value("description", "");
    
    if (j.contains("value")) {
        value = j["value"];
    }
    
    unit = j.value("unit", "");
}

// RunQuality implementation
RunQuality::RunQuality(const std::string& label,
                      const std::vector<std::shared_ptr<InputFile>>& inputFiles,
                      const std::vector<std::shared_ptr<AnalysisSoftware>>& analysisSoftware,
                      const std::vector<std::shared_ptr<QualityMetric>>& metrics)
    : label(label), inputFiles(inputFiles), analysisSoftware(analysisSoftware), metrics(metrics) {}

nlohmann::json RunQuality::toJson() const {
    nlohmann::json j;
    j["label"] = label;
    
    j["inputFiles"] = nlohmann::json::array();
    for (const auto& file : inputFiles) {
        j["inputFiles"].push_back(file->toJson());
    }
    
    j["analysisSoftware"] = nlohmann::json::array();
    for (const auto& software : analysisSoftware) {
        j["analysisSoftware"].push_back(software->toJson());
    }
    
    j["metrics"] = nlohmann::json::array();
    for (const auto& metric : metrics) {
        j["metrics"].push_back(metric->toJson());
    }
    
    return j;
}

void RunQuality::fromJson(const nlohmann::json& j) {
    label = j.value("label", "");
    
    // Parse input files
    if (j.contains("inputFiles") && j["inputFiles"].is_array()) {
        inputFiles.clear();
        for (const auto& file : j["inputFiles"]) {
            auto inputFile = std::make_shared<InputFile>();
            inputFile->fromJson(file);
            inputFiles.push_back(inputFile);
        }
    }
    
    // Parse analysis software
    if (j.contains("analysisSoftware") && j["analysisSoftware"].is_array()) {
        analysisSoftware.clear();
        for (const auto& software : j["analysisSoftware"]) {
            auto sw = std::make_shared<AnalysisSoftware>();
            sw->fromJson(software);
            analysisSoftware.push_back(sw);
        }
    }
    
    // Parse metrics
    if (j.contains("metrics") && j["metrics"].is_array()) {
        metrics.clear();
        for (const auto& metric : j["metrics"]) {
            auto qualityMetric = std::make_shared<QualityMetric>();
            qualityMetric->fromJson(metric);
            metrics.push_back(qualityMetric);
        }
    }
}

// SetQuality implementation
SetQuality::SetQuality(const std::string& label,
                      const std::vector<std::string>& setRefs,
                      const std::vector<std::shared_ptr<QualityMetric>>& metrics)
    : label(label), setRefs(setRefs), metrics(metrics) {}

nlohmann::json SetQuality::toJson() const {
    nlohmann::json j;
    j["label"] = label;
    j["setRefs"] = setRefs;
    
    j["metrics"] = nlohmann::json::array();
    for (const auto& metric : metrics) {
        j["metrics"].push_back(metric->toJson());
    }
    
    return j;
}

void SetQuality::fromJson(const nlohmann::json& j) {
    label = j.value("label", "");
    
    // Parse setRefs
    if (j.contains("setRefs") && j["setRefs"].is_array()) {
        setRefs.clear();
        for (const auto& ref : j["setRefs"]) {
            setRefs.push_back(ref.get<std::string>());
        }
    }
    
    // Parse metrics
    if (j.contains("metrics") && j["metrics"].is_array()) {
        metrics.clear();
        for (const auto& metric : j["metrics"]) {
            auto qualityMetric = std::make_shared<QualityMetric>();
            qualityMetric->fromJson(metric);
            metrics.push_back(qualityMetric);
        }
    }
}

// MzQcFile implementation
MzQCFile::MzQCFile(const std::string& creationDate,
                   const std::string& version,
                   const std::string& contactName,
                   const std::string& contactAddress,
                   const std::string& description,
                   const std::vector<std::shared_ptr<RunQuality>>& runQualities,
                   const std::vector<std::shared_ptr<SetQuality>>& setQualities)
    : creationDate(creationDate.empty() ? getCurrentIsoTime() : creationDate),
      version(version),
      contactName(contactName),
      contactAddress(contactAddress),
      description(description),
      runQualities(runQualities),
      setQualities(setQualities) {}

std::string MzQCFile::getCurrentIsoTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

nlohmann::json MzQCFile::toJson() const {
    nlohmann::json j;
    j["mzQC"] = {
        {"version", version},
        {"creationDate", creationDate}
    };

    if (!contactName.empty()) j["mzQC"]["contactName"] = contactName;
    if (!contactAddress.empty()) j["mzQC"]["contactAddress"] = contactAddress;
    if (!description.empty()) j["mzQC"]["description"] = description;

    if (!controlledVocabularies.empty()) {
        j["mzQC"]["controlledVocabularies"] = nlohmann::json::array();
        for (const auto& cv : controlledVocabularies) {
            j["mzQC"]["controlledVocabularies"].push_back(cv->toJson());
        }
    }

    if (!runQualities.empty()) {
        j["mzQC"]["runQualities"] = nlohmann::json::array();
        for (const auto& quality : runQualities) {
            j["mzQC"]["runQualities"].push_back(quality->toJson());
        }
    }

    if (!setQualities.empty()) {
        j["mzQC"]["setQualities"] = nlohmann::json::array();
        for (const auto& quality : setQualities) {
            j["mzQC"]["setQualities"].push_back(quality->toJson());
        }
    }

    return j;
}

void MzQCFile::fromJson(const nlohmann::json& j) {
    // Check if the root object is "mzQC" or if we're already inside it
    const auto& mzqc = j.contains("mzQC") ? j["mzQC"] : j;
    
    // Parse basic properties
    if (mzqc.contains("creationDate")) {
        creationDate = mzqc["creationDate"].get<std::string>();
    } else {
        creationDate = getCurrentIsoTime();



    }
    
    version = mzqc.value("version", "");
    contactName = mzqc.value("contactName", "");
    contactAddress = mzqc.value("contactAddress", "");
    description = mzqc.value("description", "");
    
    // Parse controlled vocabularies
    if (mzqc.contains("controlledVocabularies") && mzqc["controlledVocabularies"].is_array()) {
        controlledVocabularies.clear();
        for (const auto& v : mzqc["controlledVocabularies"]) {
            auto cv = std::make_shared<ControlledVocabulary>();
            cv->fromJson(v);
            controlledVocabularies.push_back(cv);
        }
    }
    
    // Parse run qualities
    if (mzqc.contains("runQualities") && mzqc["runQualities"].is_array()) {
        runQualities.clear();
        for (const auto& rq : mzqc["runQualities"]) {
            auto runQuality = std::make_shared<RunQuality>();
            runQuality->fromJson(rq);
            runQualities.push_back(runQuality);
        }
    }
    
    // Parse set qualities
    if (mzqc.contains("setQualities") && mzqc["setQualities"].is_array()) {
        setQualities.clear();
        for (const auto& sq : mzqc["setQualities"]) {
            auto setQuality = std::make_shared<SetQuality>();
            setQuality->fromJson(sq);
            setQualities.push_back(setQuality);
        }
    }
}

std::shared_ptr<MzQCFile> MzQCFile::fromJsonStatic(const nlohmann::json& j) {
    auto file = std::make_shared<MzQCFile>();
    file->fromJson(j);
    return file;
}

std::shared_ptr<MzQCFile> MzQCFile::fromFile(const std::string& filepath, const std::string& schemaPath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Error parsing JSON from file: " + std::string(e.what()));
    }
    
    // Validate against schema if schema path is provided
    if (!schemaPath.empty()) {
        if (!validateAgainstSchema(j, schemaPath)) {
            throw std::runtime_error("File does not conform to mzQC schema");
        }
    }
    
    return fromJsonStatic(j);
}

void MzQCFile::toFile(const std::string& filepath, const std::string& schemaPath) const {
    nlohmann::json j = toJson();
    
    // Validate against schema if schema path is provided
    if (!schemaPath.empty()) {
        if (!validateAgainstSchema(j, schemaPath)) {
            throw std::runtime_error("Generated mzQC does not conform to schema");
        }
    }
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filepath);
    }

    file << j.dump(2);
}

} // namespace mzqc 