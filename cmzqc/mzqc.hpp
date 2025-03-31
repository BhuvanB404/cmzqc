#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <map>
#include <fstream>
#include <nlohmann/json.hpp>

namespace mzqc {




class CvParameter;
class ControlledVocabulary;
class CvTermDetails;
class CvTermCache;
class InputFile;
class AnalysisSoftware;
class QualityMetric;
class RunQuality;
class SetQuality;
class MzQcFile;
class JsonSerializable;

// Function declarations for schema validation
nlohmann::json loadSchema(const std::string& schemaPath);
bool validateAgainstSchema(const nlohmann::json& j, const std::string& schemaPath);

// Base class for JSON serialization
class JsonSerializable {
public:
    virtual ~JsonSerializable() = default;
    virtual nlohmann::json toJson() const = 0;
    virtual void fromJson(const nlohmann::json& j) = 0;
};

// CvTermDetails class
class CvTermDetails {
public:
    CvTermDetails() = default;
    
    std::string accession;     // MS ID
    std::string name;          // Human readable name
    std::string definition;
    std::vector<std::string> relationships; // ontology relationship
    std::vector<std::string> parentTerms;
    std::optional<std::string> valueType;
    std::optional<std::string> unit;
};

// CvTermCache class
class CvTermCache {
public:
    CvTermCache() = default;
    
    int loadFromOboFile(const std::string& filename);
    int parseOboFile(const std::string& filename);
    

    
private:
    std::string currentOboFile;
    std::map<std::string, CvTermDetails> termCache;
};

// ControlledVocabulary class
class ControlledVocabulary : public JsonSerializable {
public:
    ControlledVocabulary(const std::string& name = "",
                         const std::string& uri = "",
                         const std::string& version = "");
    
    std::string id;
    std::string name;
    std::string uri;
    std::string version;
    
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& j) override;
};

// From PDF: CvParameter class
class CvParameter : public JsonSerializable {
public:
    CvParameter(const std::string& accession = "",
                const std::string& name = "",
                const std::string& value = "",
                const std::string& cvRef = "");

    std::string accession;
    std::string name;
    std::string value;
    std::string cvRef;

    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& j) override;
};

// From PDF: AnalysisSoftware class
class AnalysisSoftware : public JsonSerializable {
public:
    AnalysisSoftware(const std::string& accession = "",
                     const std::string& name = "",
                     const std::string& version = "",
                     const std::string& uri = "");

    std::string accession;
    std::string name;
    std::string version;
    std::string uri;

    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& j) override;
};

// From PDF: InputFile class
class InputFile : public JsonSerializable {
public:
    InputFile(const std::string& location = "",
              const std::string& name = "",
              const std::shared_ptr<CvParameter>& fileFormat = nullptr,
              const std::vector<std::shared_ptr<CvParameter>>& fileProperties = {});

    std::string location;
    std::string name;
    std::shared_ptr<CvParameter> fileFormat;
    std::vector<std::shared_ptr<CvParameter>> fileProperties;

    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& j) override;
};

// From PDF: QualityMetric class
class QualityMetric : public JsonSerializable {
public:
    QualityMetric(const std::string& accession = "",
                  const std::string& name = "",
                  const std::string& description = "",
                  const nlohmann::json& value = nullptr,
                  const std::string& unit = "");

    std::string accession;
    std::string name;
    std::string description;
    nlohmann::json value;
    std::string unit;

    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& j) override;
};

// From PDF: RunQuality class
class RunQuality : public JsonSerializable {
public:
    RunQuality(const std::string& label = "",
               const std::vector<std::shared_ptr<InputFile>>& inputFiles = {},
               const std::vector<std::shared_ptr<AnalysisSoftware>>& analysisSoftware = {},
               const std::vector<std::shared_ptr<QualityMetric>>& metrics = {});

    std::string label;
    std::vector<std::shared_ptr<InputFile>> inputFiles;
    std::vector<std::shared_ptr<AnalysisSoftware>> analysisSoftware;
    std::vector<std::shared_ptr<QualityMetric>> metrics;

    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& j) override;
};

// From PDF: SetQuality class
class SetQuality : public JsonSerializable {
public:
    SetQuality(const std::string& label = "",
               const std::vector<std::string>& setRefs = {},
               const std::vector<std::shared_ptr<QualityMetric>>& metrics = {});

    std::string label;
    std::vector<std::string> setRefs;
    std::vector<std::shared_ptr<QualityMetric>> metrics;

    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& j) override;
};

// From PDF: MzQcFile class
class MzQcFile : public JsonSerializable {
public:
    MzQcFile(const std::string& creationDate = "",
             const std::string& version = "1.0.0",
             const std::string& contactName = "",
             const std::string& contactAddress = "",
             const std::string& description = "",
             const std::vector<std::shared_ptr<RunQuality>>& runQualities = {},
             const std::vector<std::shared_ptr<SetQuality>>& setQualities = {});

    std::string creationDate;
    std::string version;
    std::string contactName;
    std::string contactAddress;
    std::string description;
    std::vector<std::shared_ptr<ControlledVocabulary>> controlledVocabularies;
    std::vector<std::shared_ptr<RunQuality>> runQualities;
    std::vector<std::shared_ptr<SetQuality>> setQualities;

    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& j) override;
    static std::shared_ptr<MzQcFile> fromJsonStatic(const nlohmann::json& j);
    static std::shared_ptr<MzQcFile> fromFile(const std::string& filepath, const std::string& schemaPath = "");
    void toFile(const std::string& filepath, const std::string& schemaPath = "") const;

    static std::string getCurrentIsoTime();
};

} // namespace mzqc 