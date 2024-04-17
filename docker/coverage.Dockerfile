### Generate coverage report container ###
# Define coverage stage
FROM raddish:base as coverage

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build_coverage

# Build 
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage
# RUN ctest --output-on_failure

