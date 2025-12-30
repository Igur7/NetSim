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

//dodawanie elementu do fabryki
TEST(FactoryTest, AddRampIncreasesRampCount) {
    Factory f;
    f.add_ramp(Ramp(1, 1));

    EXPECT_EQ(std::distance(f.ramp_cbegin(), f.ramp_cend()), 1);
}



//polaczenie reciever-worker po remove_worker
TEST(FactoryTest, RemovingWorkerMakesFactoryInconsistent) {
    Factory f;

    f.add_ramp(Ramp(1, 1));
    f.add_worker(
        Worker(
            1,
            1,
            std::make_unique<PackageQueue>(PackageQueueType::Fifo)
        )
    );
    f.add_storehouse(Storehouse(1));

    f.remove_worker(1);

    EXPECT_FALSE(f.is_consistent());
}

//is consistent bez polacenia storehouse (exp: False)
//is consistent z możliwą błędną drogą (exp: False)
TEST(FactoryTest, IsConsistent_MixedWithCycle) {
    // R -> W1 -> S
    //      W1 -> W2 -> W2
    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_worker(Worker(2, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1));

    Ramp& r = *(factory.find_ramp_by_id(1));
    r.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(1)));

    Worker& w1 = *(factory.find_worker_by_id(1));
    w1.receiver_preferences_.add_receiver(&(*factory.find_storehouse_by_id(1)));
    w1.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(2)));

    Worker& w2 = *(factory.find_worker_by_id(2));
    w2.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(2))); // sam do siebie

    EXPECT_FALSE(factory.is_consistent());
}
//czy usuwanie workera uniemożliwia jego wylosowanie
TEST(FactoryTest, RemoveWorkerTwoRemainingReceivers) {
    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    factory.add_worker(Worker(2, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    factory.add_worker(Worker(3, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));

    Ramp& r = *(factory.find_ramp_by_id(1));
    r.receiver_preferences_.add_receiver(&(*(factory.find_worker_by_id(1))));
    r.receiver_preferences_.add_receiver(&(*(factory.find_worker_by_id(2))));
    r.receiver_preferences_.add_receiver(&(*(factory.find_worker_by_id(3))));


    factory.remove_worker(1);

    auto prefs = r.receiver_preferences_.get_preferences();
    ASSERT_EQ(prefs.size(), 2U);

    auto it2 = prefs.find(&(*(factory.find_worker_by_id(2))));
    ASSERT_NE(it2, prefs.end());
    EXPECT_DOUBLE_EQ(it2->second, 1.0 / 2.0);

    auto it3 = prefs.find(&(*(factory.find_worker_by_id(3))));
    ASSERT_NE(it3, prefs.end());
    EXPECT_DOUBLE_EQ(it3->second, 1.0 / 2.0);
}
//usuwanie nieistniejących workerów
TEST(NodeCollectionTest, RemoveNonExistingIsNoOp) {
    NodeCollection<Worker> col;
    col.add(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    col.add(Worker(2, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));


    size_t count_before = 0;
    for (auto it = col.cbegin(); it != col.cend(); ++it) ++count_before;


    col.remove_by_id(999);

    size_t count_after = 0;
    for (auto it = col.cbegin(); it != col.cend(); ++it) ++count_after;

    EXPECT_EQ(count_before, count_after);
}
