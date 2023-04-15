
# Start from the latest Debian image
FROM debian:latest

# Update the package repository and install dependencies
RUN apt-get update && \
    apt-get install -y curl gcc make tar

# Download and install Siege
RUN curl -L http://download.joedog.org/siege/siege-latest.tar.gz | tar zx && \
    cd siege-* && \
    ./configure && \
    make && \
    make install

ENTRYPOINT ["siege"]
# Set the default command for the container
CMD ["-c", "3", "-r", "1", "10.11.9.7:6969"]