### Generate coverage report container ###
# Define coverage stage
FROM raddish:base as coverage

# Share work directory
COPY . /usr/src/project

# Submodule init
WORKDIR /usr/src/project
RUN git submodule update --init --recursive

# Go to build coverage
WORKDIR /usr/src/project/build_coverage

# Build 
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage
# RUN ctest --output-on_failure

