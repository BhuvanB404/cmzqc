# mzQC Reader & Writer Library

A C++ library for reading, writing, and manipulating mzQC files for mass spectrometry quality control data. This library implements the PSI mzQC standard for representing quality control metrics for mass spectrometry experiments.

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Docker Setup](#docker-setup)
- [Library Usage](#library-usage)
- [API Documentation](#api-documentation)
- [Building Manually](#building-manually)
- [Examples](#examples)


## 🔍 Overview

The mzQC library provides a standardized way to represent, store, and share quality control metrics for mass spectrometry proteomics data. It supports the PSI (Proteomics Standards Initiative) mzQC format, which is designed to facilitate the exchange of quality control information between different tools and laboratories.

## ✨ Features

- **Read & Write mzQC Files**: Easily parse and generate mzQC JSON files
- **Schema Validation**: Validate mzQC files against the official schema
- **Controlled Vocabulary Support**: Work with PSI-MS and QC controlled vocabularies
- **Quality Metrics**: Create, manipulate, and analyze quality metrics for MS experiments
- **Run & Set Quality**: Handle both individual run quality metrics and set-level metrics
- **C++ API**: Modern C++17 interface with clean abstractions

## 🐳 Docker Setup

### Prerequisites

- [Docker](https://www.docker.com/get-started) installed on your system

### Building the Docker Image

```bash
# Clone the repository (if you haven't already)
git clone https://github.com/yourusername/cmzqc.git
cd cmzqc

# Build the Docker image
docker build -t cmzqc-POC .
```

### Running the Container

```bash
# Run the example application (default)
docker run --rm mzqc-reader

# Run with an interactive shell
docker run -it --rm mzqc-reader bash
```

### Working with Your Data

To process your own files, you can mount a local directory to the container:

```bash
# Mount a local directory to /data in the container
docker run -it -v $(pwd)/your_data:/data --rm mzqc-reader bash

# Inside the container, you can access your files
cd /data
# Run commands as needed
```

### Docker Container Structure

- `/app`: Root directory containing the source code
- `/app/build`: Build directory with compiled executables
  - `example`: Example application
  - `mzqc_reader`: Main mzQC reader application

## 📚 Library Usage

### Basic Example

Here's a simple example of how to create and write an mzQC file:

```cpp
#include "mzqc.hpp"
#include <memory>
#include <iostream>

int main() {
    // Create input file
    auto inputFile = std::make_shared<mzqc::InputFile>(
        "file:///path/to/input.mzML", 
        "input.mzML",
        std::make_shared<mzqc::CvParameter>("MS:1000584", "mzML file", "", "PSI-MS")
    );
    
    // Create analysis software
    auto software = std::make_shared<mzqc::AnalysisSoftware>(
        "MS:1000799", 
        "custom tool", 
        "1.0.0", 
        "http://example.org/tool"
    );

    // Create a quality metric
    auto metric = std::make_shared<mzqc::QualityMetric>(
        "QC:4000053", 
        "RT duration", 
        "Total retention time duration", 
        60.5, 
        "UO:0000031"
    );

    // Create controlled vocabulary entries
    auto cv1 = std::make_shared<mzqc::ControlledVocabulary>(
        "PSI-MS", 
        "https://github.com/HUPO-PSI/psi-ms-CV/blob/master/psi-ms.obo", 
        "4.1.55"
    );
    
    auto cv2 = std::make_shared<mzqc::ControlledVocabulary>(
        "QC", 
        "https://github.com/HUPO-PSI/qcML-development/blob/master/cv/qc-cv.obo", 
        "0.1.0"
    );

    // Create run quality
    auto runQuality = std::make_shared<mzqc::RunQuality>(
        "Example Run", 
        std::vector<std::shared_ptr<mzqc::InputFile>>{inputFile},
        std::vector<std::shared_ptr<mzqc::AnalysisSoftware>>{software},
        std::vector<std::shared_ptr<mzqc::QualityMetric>>{metric}
    );

    // Create mzQC file
    mzqc::MzQcFile mzqcFile(
        "2023-04-01T12:00:00", 
        "1.0.0", 
        "John Doe", 
        "john.doe@example.com", 
        "Example mzQC file",
        std::vector<std::shared_ptr<mzqc::RunQuality>>{runQuality}, 
        std::vector<std::shared_ptr<mzqc::SetQuality>>{}
    );
    
    // Add controlled vocabularies
    mzqcFile.controlledVocabularies = {cv1, cv2};
    
    // Write to file
    std::ofstream outFile("output.mzqc");
    outFile << mzqcFile.toJson().dump(4);
    outFile.close();
    
    std::cout << "mzQC file generated successfully." << std::endl;
    return 0;
}
```

### Reading an mzQC File

```cpp
#include "mzqc.hpp"
#include <iostream>
#include <fstream>

int main() {
    // Read mzQC file
    std::ifstream file("input.mzqc");
    if (!file.is_open()) {
        std::cerr << "Failed to open mzQC file." << std::endl;
        return 1;
    }
    
    nlohmann::json j;
    file >> j;
    file.close();
    
    // Create MzQcFile object from JSON
    mzqc::MzQcFile mzqcFile;
    mzqcFile.fromJson(j);
    
    // Access data
    std::cout << "Contact name: " << mzqcFile.contactName << std::endl;
    std::cout << "Number of run qualities: " << mzqcFile.runQualities.size() << std::endl;
    
    // Access metrics
    if (!mzqcFile.runQualities.empty() && !mzqcFile.runQualities[0]->metrics.empty()) {
        auto& metric = mzqcFile.runQualities[0]->metrics[0];
        std::cout << "First metric: " << metric->name << " = " << metric->value << std::endl;
    }
    
    return 0;
}
```

## 📖 API Documentation

### Key Classes

- **MzQcFile**: The main container for mzQC data
- **RunQuality**: Represents quality metrics for a single MS run
- **SetQuality**: Represents quality metrics for a set of MS runs
- **QualityMetric**: Represents a single quality metric
- **InputFile**: Represents an input file reference
- **AnalysisSoftware**: Represents software used in the analysis
- **CvParameter**: Represents a controlled vocabulary parameter
- **ControlledVocabulary**: Represents a controlled vocabulary source

## 🛠️ Building Manually

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10+
- nlohmann_json library (automatically fetched if not found)

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yourusername/cmzqc.git
cd cmzqc

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make

# Run the example
./example
```

## 📊 Examples

The repository includes an example application (`example.cpp`) that demonstrates how to:

1. Read data from a CSV file
2. Create quality metrics from the data
3. Set up an mzQC file with run qualities
4. Write the mzQC file to disk

To run the example:

```bash
# If using Docker
docker run --rm mzqc-reader

# If building manually
cd build
./example
```

## Acknowledgements

- [HUPO-PSI](https://www.psidev.info/) for the mzQC standard
- [nlohmann/json](https://github.com/nlohmann/json) for JSON parsing 