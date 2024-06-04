#include <iostream>
#include <limits>
using namespace std;

const int TABLE_SIZE = 100;

class Timestamp {
public:
    int hour;
    int min;
    int sec;
    
    Timestamp(int hour = 0, int min = 0 , int sec = 0)
     : hour(hour), min(min), sec(sec)
      {}

    bool operator<(const Timestamp& other) const {
        if (hour != other.hour) return hour < other.hour;
        if (min != other.min) return min < other.min;
        return sec < other.sec;
    }
    
    bool operator>(const Timestamp& other) const {
        if (hour != other.hour) return hour > other.hour;
        if (min != other.min) return min > other.min;
        return sec > other.sec;
    }
    
    bool operator==(const Timestamp& other) const {
        return hour == other.hour && min == other.min && sec == other.sec;
    }
    
    bool operator!=(const Timestamp& other) const {
        return !(*this == other);
    }
};

struct Data {
    Timestamp timestamp;
    double value;
    Data* next;
};

struct Node {
    Timestamp timestamp;
    double value;
    Node* left;
    Node* right;
    Node(Timestamp timestamp, double value = 0)
     : timestamp(timestamp), value(value), left(nullptr), right(nullptr) 
     {}
};

class LinkedList {
public:
    Data* head;
    LinkedList() : head(nullptr) {}
    
    void insert(Timestamp timestamp, double value) {
        Data* newNode = new Data{timestamp, value, head};
        head = newNode;
    }
    
    void remove(Timestamp timestamp) {
        Data* temp = head;
        Data* prev = nullptr;
        while (temp != nullptr && temp->timestamp != timestamp) {
            prev = temp;
            temp = temp->next;
        }
        if (temp != nullptr) {
            if (prev != nullptr) {
                prev->next = temp->next;
            } else {
                head = temp->next;
            }
            delete temp;
        }
    }
    
    Data* find(Timestamp timestamp) {
        Data* temp = head;
        while (temp != nullptr) {
            if (temp->timestamp == timestamp) {
                return temp;
            }
            temp = temp->next;
        }
        return nullptr;
    }
};

class HashTable {
public:
    LinkedList table[TABLE_SIZE];
    
    int hashFunction(int sensor_id) {
        return sensor_id % TABLE_SIZE;
    }
    
    LinkedList& getLinkedList(int sensor_id) {
        int index = hashFunction(sensor_id);
        return table[index];
    }
};

class BST {
public:
    Node* root;
    
    BST() : root(nullptr) {}
    
    Node* insert(Node* node, Timestamp timestamp, double value) {
        if (!node) {
            return new Node(timestamp, value);
        }
        if (timestamp < node->timestamp) {
            node->left = insert(node->left, timestamp, value);
        } else if (timestamp > node->timestamp) {
            node->right = insert(node->right, timestamp, value);
        }
        return node;
    }
    
    void insert(Timestamp timestamp, double value) {
        root = insert(root, timestamp, value);
    }
    
    Node* find(Node* node, Timestamp timestamp) {
        if (!node || node->timestamp == timestamp) {
            return node;
        }
        if (timestamp < node->timestamp) {
            return find(node->left, timestamp);
        } else {
            return find(node->right, timestamp);
        }
    }
    
    Node* find(Timestamp timestamp) {
        return find(root, timestamp);
    }
    
    Node* findMin(Node* node) {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }
    
    Node* remove(Node* node, Timestamp timestamp) {
        if (!node) return node;
        if (timestamp < node->timestamp) {
            node->left = remove(node->left, timestamp);
        } else if (timestamp > node->timestamp) {
            node->right = remove(node->right, timestamp);
        } else {
            if (!node->left) {
                Node* temp = node->right;
                delete node;
                return temp;
            } else if (!node->right) {
                Node* temp = node->left;
                delete node;
                return temp;
            }
            Node* temp = findMin(node->right);
            node->timestamp = temp->timestamp;
            node->value = temp->value;
            node->right = remove(node->right, temp->timestamp);
        }
        return node;
    }
    
    void remove(Timestamp timestamp) {
        root = remove(root, timestamp);
    }
    
    void rangeQuery(Node* node, Timestamp t_start, Timestamp t_end, double& sum, double& minVal, double& maxVal, int& count) {
        if (!node) return;
        if (t_start < node->timestamp) {
            rangeQuery(node->left, t_start, t_end, sum, minVal, maxVal, count);
        }
        if ((t_start < node->timestamp || t_start == node->timestamp )&& (t_end > node->timestamp || t_end == node->timestamp)){
            sum += node->value;
            minVal = min(minVal, node->value);
            maxVal = max(maxVal, node->value);
            count++;
        }
        if (t_end > node->timestamp) {
            rangeQuery(node->right, t_start, t_end, sum, minVal, maxVal, count);
        }
    }
    
    void analyze(Timestamp t_start, Timestamp t_end, double& avg, double& minVal, double& maxVal) {
        double sum = 0;
        int count = 0;
        minVal = numeric_limits<double>::max();
        maxVal = numeric_limits<double>::lowest();
        rangeQuery(root, t_start, t_end, sum, minVal, maxVal, count);
        avg = (count == 0) ? 0 : sum / count;
    }
};

struct SensorStats {
    double avg;
    double minVal;
    double maxVal;
};

struct DataPoint {
    Timestamp timestamp;
    double value;
    DataPoint(Timestamp timestamp = {0,0,0}, double value = 0)
     : timestamp(timestamp), value(value)
     {}
};

class SensorDataManager {
private:
    HashTable sensorTable;
    
public:
    void addDataPoint(int sensor_id, Timestamp timestamp, double value) {
        LinkedList& list = sensorTable.getLinkedList(sensor_id);
        if (list.find(timestamp)) {
            list.remove(timestamp);
        }
        list.insert(timestamp, value);
        BST* bst = new BST();
        bst->insert(timestamp, value);
    }
    
    void updateDataPoint(int sensor_id, Timestamp timestamp, double value) {
        LinkedList& list = sensorTable.getLinkedList(sensor_id);
        Data* dataPoint = list.find(timestamp);
        if (dataPoint) {
            dataPoint->value = value;
        }
    }
    
    void deleteDataPoints(int sensor_id, Timestamp t_old) {
        LinkedList& list = sensorTable.getLinkedList(sensor_id);
        Data* curr = list.head;
        while (curr) {
            if (curr->timestamp < t_old) {
                list.remove(curr->timestamp);
            }
            curr = curr->next;
        }
    }
    
    void retrieveData(int sensor_id, Timestamp t_start, Timestamp t_end, DataPoint* result, int& count) {
        LinkedList& list = sensorTable.getLinkedList(sensor_id);
        count = 0;
        Data* curr = list.head;
        while (curr != nullptr) {
            if (curr->timestamp > t_start && curr->timestamp < t_end) {
                result[count++] = {curr->timestamp, curr->value};
            }
            curr = curr->next;
        }
    }
    
    SensorStats analyzeSensorTrends(int sensor_id, Timestamp t_start, Timestamp t_end) {
        LinkedList& list = sensorTable.getLinkedList(sensor_id);
        Data* curr = list.head;
        BST* bst = new BST();
        while (curr) {
            bst->insert(curr->timestamp, curr->value);
            curr = curr->next;
        }
        SensorStats stats;
        bst->analyze(t_start, t_end, stats.avg, stats.minVal, stats.maxVal);
        return stats;
    }
};

int main() {
    SensorDataManager manager;

    manager.addDataPoint(1, {12, 30, 23}, 25.3);
    manager.addDataPoint(1, {12, 33, 3}, 26.1);
    manager.addDataPoint(1, {12, 35, 43}, 24.8);
    manager.addDataPoint(2, {12, 34, 13}, 22.3);
    manager.addDataPoint(2, {12, 46, 9}, 21.9);
    manager.addDataPoint(2, {12, 42, 23}, 14.3);
    
    DataPoint data[10];
    int count;
    manager.retrieveData(1, {12, 30, 0}, {12, 35, 0}, data, count);
    cout << "Data from sensor 1 (12:30:00 - 12:35:00) are " << count << endl;
    for (int i = 0; i < count; ++i) {
        cout << "Timestamp: " << data[i].timestamp.hour << ":" << data[i].timestamp.min << ":" << data[i].timestamp.sec << ", Value: " << data[i].value << endl;
    }


    DataPoint data2[10];
    int count2;
    manager.retrieveData(2, {12, 40, 0}, {12, 50, 0}, data2, count2);
    cout << "Data from sensor 1 (12:40:00 - 12:50:00) are " << count2 << endl;
    for (int i = 0; i < count2; ++i) {
        cout << "Timestamp: " << data[i].timestamp.hour << ":" << data[i].timestamp.min << ":" << data[i].timestamp.sec << ", Value: " << data[i].value << endl;
    }

    manager.updateDataPoint(1, {12, 39,3}, 27.2);

    DataPoint data3[10];
    int count3;
    manager.retrieveData(1, {12, 30, 0}, {12, 35, 0}, data3, count3);
    cout << "\nAfter updating\nData from sensor 1 (12:40:00 - 12:50:00) are " << count3 << endl;
    for (int i = 0; i < count3; ++i) {
        cout << "Timestamp: " << data[i].timestamp.hour << ":" << data[i].timestamp.min << ":" << data[i].timestamp.sec << ", Value: " << data[i].value << endl;
    }
    SensorStats stats = manager.analyzeSensorTrends(1, {12, 30, 0}, {12, 50, 0});
    cout << "\nAverage: " << stats.avg << ", Min: " << stats.minVal << ", Max: " << stats.maxVal << endl;

    manager.deleteDataPoints(1, {12, 35, 0});

    return 0;
}

