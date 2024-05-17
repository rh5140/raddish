#include "crud_store.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>

CRUDStore::CRUDStore(std::string data_path) : data_path_(data_path) {
    // no checks need to be done on the data path
    // since the crud handler checks this before passing it in here
}

int CRUDStore::create(std::string entity_type, std::string post_body) {
    std::string entity_path = data_path_ + "/" + entity_type;
    if (!std::filesystem::exists(entity_path)) {
        std::filesystem::create_directories(entity_path);
    }

    // cheeky code to get around needing a private variable to store the id
    // and having to maintain that variable state
    int new_id = 1;
    std::vector<int> ids = list(entity_type);
    if (!ids.empty()) {
        new_id = *std::max_element(ids.begin(), ids.end()) + 1;
    }

    std::string file_path = entity_path + "/" + std::to_string(new_id);
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return -1;
    }

    file << post_body;
    file.close();
    return new_id;
}

std::optional<std::string> CRUDStore::retrieve(std::string entity_type, int id) {
    std::string file_path = data_path_ + "/" + entity_type + "/" + std::to_string(id);
    if (!std::filesystem::exists(file_path)) {
        return std::nullopt;
    }

    std::ifstream file(file_path);
    if (!file.is_open()) {
        return std::nullopt;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

bool CRUDStore::update(std::string entity_type, int id, std::string put_body) {
    std::string file_path = data_path_ + "/" + entity_type + "/" + std::to_string(id);
    if (!std::filesystem::exists(file_path)) {
        return false;
    }

    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }

    file << put_body;
    file.close();
    return true;
}

bool CRUDStore::del(std::string entity_type, int id) {
    std::string file_path = data_path_ + "/" + entity_type + "/" + std::to_string(id);
    return std::filesystem::remove(file_path);
}

std::vector<int> CRUDStore::list(std::string entity_type) {
    std::string entity_path = data_path_ + "/" + entity_type;
    std::vector<int> ids;

    if (!std::filesystem::exists(entity_path)) {
        return ids;
    }

    for (const auto& entry : std::filesystem::directory_iterator(entity_path)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            try {
                int id = std::stoi(filename);
                ids.push_back(id);
            } catch (const std::invalid_argument&) {
                // ignoring non-integer filenames, this will never happen
                // unless someone manually creates them in data_path_
                // please don't do that, that's silly
            }
        }
    }
    
    std::sort(ids.begin(), ids.end());
    return ids;
}
