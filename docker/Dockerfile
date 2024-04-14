### Build/test container ###
# Define builder stage
FROM raddish:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:jammy as deploy

# Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/webserver .

# Copy config file
COPY --from=builder /usr/src/project/server_config .

# Expose ports
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
# Default config file
CMD ["server_config"]
