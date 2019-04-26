# docker compiler and run env
# for judge end.
# if you living in china, you should using vpn or anything else to across the wall
#  otherwise may not be able to build this images.

FROM ubuntu:18.10
MAINTAINER linkcheng "linkjieshi@gmail.com"
ENV REFRESHED_AT 2019-4-25

# install
# make; vim; gcc; g++; git
# gcc; g++ requirement up to 7.2
RUN apt-get update && apt-get -y install gcc make

# download guatOJ 
WORKDIR /root/repo/
