#include "gtest/gtest.h"
#include "factory.hpp"
#include "nodes.hpp"
#include "types.hpp"

TEST(NodeCollectionTest, FindById) {
    // Arrange
    NodeCollection<Ramp> collection;
    collection.add(Ramp(1, 1)); // ID=1, delivery_interval=1
    collection.add(Ramp(2, 1)); // ID=2, delivery_interval=1

    // Act
    auto it1 = collection.find_by_id(1);
    auto it2 = collection.find_by_id(2);
    auto it3 = collection.find_by_id(3); // Element nieistniejący

    // Assert
    ASSERT_NE(it1, collection.end()); // Znaleziono ID 1
    EXPECT_EQ(it1->get_id(), 1);

    ASSERT_NE(it2, collection.end()); // Znaleziono ID 2
    EXPECT_EQ(it2->get_id(), 2);

    EXPECT_EQ(it3, collection.end()); // Nie znaleziono ID 3
}

//Usuwanie po ID

TEST(NodeCollectionTest, RemoveById) {
    // Arrange
    NodeCollection<Ramp> collection;
    collection.add(Ramp(1, 1));
    collection.add(Ramp(2, 1));

    // Act
    collection.remove_by_id(1); 

    // Assert
    auto it1 = collection.find_by_id(1);
    auto it2 = collection.find_by_id(2);

    EXPECT_EQ(it1, collection.end()); 
    ASSERT_NE(it2, collection.end()); 
    EXPECT_EQ(it2->get_id(), 2);      
}
