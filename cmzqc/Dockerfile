FROM ubuntu:22.04


ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    g++ \
    make \
    curl \
    nlohmann-json3-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy project files
COPY . .

# Create build directory
RUN mkdir -p build

# Build the project
WORKDIR /app/build
RUN cmake .. && make

# Add schema to the build directory if not already copied
RUN if [ ! -f mzqc_schema.json ]; then cp ../schema/mzqc_schema.json .; fi

# Copy the CSV file to make the example work
RUN if [ ! -f CPTAC_CompRef_00_iTRAQ_01_2Feb12_Cougar_11-10-09_ids.csv ]; then \
    cp ../input_files/CPTAC_CompRef_00_iTRAQ_01_2Feb12_Cougar_11-10-09_ids.csv .; \
    fi

# Command to run the example by default
CMD ["./example"]

# Additional run options as comments:
# To run the mzqc_reader: CMD ["./mzqc_reader"]
# To run with bash: CMD ["bash"] 