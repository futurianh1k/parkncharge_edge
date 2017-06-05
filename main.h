#pragma once

#include <string>
#include <unordered_map>

#include "types.h"

std::unordered_map<int, seevider::ServerDestinations_t> ServerDestinations;

bool system_check();

void writeXMLFile(std::string filename);

void loadXMLFile(std::string filename);

int main(int argc, char** argv);
