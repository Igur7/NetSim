#include "gtest/gtest.h"
#include "factory.hpp"
#include "nodes.hpp"
#include "types.hpp"

template<typename T>
std::shared_ptr<T> make_observer_ptr(T* ptr) {
    return std::shared_ptr<T>(ptr, [](T*) {
        // no-op deleter
    });
}

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

//is consistent z możliwą błędną drogą (exp: False)
TEST(FactoryTest, IsConsistent_MixedWithCycle) {
    // R -> W1 -> S
    //      W1 -> W2 -> W2

    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    factory.add_worker(Worker(2, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    factory.add_storehouse(Storehouse(1));

    Ramp& r = *factory.find_ramp_by_id(1);
    Worker& w1 = *factory.find_worker_by_id(1);
    Worker& w2 = *factory.find_worker_by_id(2);
    Storehouse& s = *factory.find_storehouse_by_id(1);

    auto w1_ptr = make_observer_ptr<IPackageReceiver>(&w1);
    auto w2_ptr = make_observer_ptr<IPackageReceiver>(&w2);
    auto s_ptr  = make_observer_ptr<IPackageReceiver>(&s);

    r.add_receiver(w1_ptr);

    w1.add_receiver(s_ptr);
    w1.add_receiver(w2_ptr);

    w2.add_receiver(w2_ptr);

    EXPECT_FALSE(factory.is_consistent());
}
//czy usuwanie workera uniemożliwia jego wylosowanie
TEST(FactoryTest, RemoveWorkerTwoRemainingReceivers) {
    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    factory.add_worker(Worker(2, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    factory.add_worker(Worker(3, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));

    Ramp& r = *factory.find_ramp_by_id(1);
    Worker& w1 = *factory.find_worker_by_id(1);
    Worker& w2 = *factory.find_worker_by_id(2);
    Worker& w3 = *factory.find_worker_by_id(3);

    auto w1_ptr = make_observer_ptr<IPackageReceiver>(&w1);
    auto w2_ptr = make_observer_ptr<IPackageReceiver>(&w2);
    auto w3_ptr = make_observer_ptr<IPackageReceiver>(&w3);

    r.add_receiver(w1_ptr);
    r.add_receiver(w2_ptr);
    r.add_receiver(w3_ptr);

    factory.remove_worker(1);

    const auto& prefs = r.get_receiver_preferences();

    ASSERT_EQ(prefs.size(), 2U);

    auto it2 = prefs.find(w2_ptr);
    ASSERT_NE(it2, prefs.end());
    EXPECT_DOUBLE_EQ(it2->second, 1.0 / 2.0);

    auto it3 = prefs.find(w3_ptr);
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

TEST(LoadFactoryTest, LoadFactoryStructureAddsElements) {
    std::istringstream input(
        "LOADING_RAMP id=1 delivery-interval=2\n"
        "WORKER id=1 processing-time=3 queue-type=FIFO\n"
        "STOREHOUSE id=1\n"
        "LINK src=ramp-1 dest=worker-1\n"
        "LINK src=worker-1 dest=store-1\n"
    );

    IO io;
    Factory factory = io.load_factory_structure(input);

    // Sprawdzenie czy elementy zostały dodane
    auto ramp_it = factory.find_ramp_by_id(1);
    ASSERT_NE(ramp_it, factory.ramp_cend());
    EXPECT_EQ(ramp_it->get_delivery_interval(), 2);

    auto worker_it = factory.find_worker_by_id(1);
    ASSERT_NE(worker_it, factory.worker_cend());
    EXPECT_EQ(worker_it->get_processing_duration(), 3);
    EXPECT_EQ(worker_it->get_queue()->getQueueType(), PackageQueueType::Fifo);

    auto storehouse_it = factory.find_storehouse_by_id(1);
    ASSERT_NE(storehouse_it, factory.storehouse_cend());

    // Sprawdzenie czy połączenia zostały utworzone
    const auto& ramp_prefs = ramp_it->get_receiver_preferences();
    ASSERT_EQ(ramp_prefs.size(), 1);
    EXPECT_EQ(ramp_prefs.begin()->first->get_id(), 1); // worker-1

    const auto& worker_prefs = worker_it->get_receiver_preferences();
    ASSERT_EQ(worker_prefs.size(), 1);
    EXPECT_EQ(worker_prefs.begin()->first->get_id(), 1); // storehouse-1
}

TEST(LoadFactoryTest, LoadFullFactoryStructure) {
    std::istringstream input(
        "; == LOADING RAMPS ==\n"
        "\n"
        "LOADING_RAMP id=1 delivery-interval=3\n"
        "LOADING_RAMP id=2 delivery-interval=2\n"
        "\n"
        "; == WORKERS ==\n"
        "\n"
        "WORKER id=1 processing-time=2 queue-type=FIFO\n"
        "WORKER id=2 processing-time=1 queue-type=LIFO\n"
        "\n"
        "; == STOREHOUSES ==\n"
        "\n"
        "STOREHOUSE id=1\n"
        "\n"
        "; == LINKS ==\n"
        "\n"
        "LINK src=ramp-1 dest=worker-1\n"
        "\n"
        "LINK src=ramp-2 dest=worker-1\n"
        "LINK src=ramp-2 dest=worker-2\n"
        "\n"
        "LINK src=worker-1 dest=worker-1\n"
        "LINK src=worker-1 dest=worker-2\n"
        "\n"
        "LINK src=worker-2 dest=store-1\n"
    );

    IO io;
    Factory factory = io.load_factory_structure(input);

    /* ===== RAMPS ===== */
    auto r1 = factory.find_ramp_by_id(1);
    auto r2 = factory.find_ramp_by_id(2);

    ASSERT_NE(r1, factory.ramp_cend());
    ASSERT_NE(r2, factory.ramp_cend());

    EXPECT_EQ(r1->get_delivery_interval(), 3);
    EXPECT_EQ(r2->get_delivery_interval(), 2);

    /* ===== WORKERS ===== */
    auto w1 = factory.find_worker_by_id(1);
    auto w2 = factory.find_worker_by_id(2);

    ASSERT_NE(w1, factory.worker_cend());
    ASSERT_NE(w2, factory.worker_cend());

    EXPECT_EQ(w1->get_processing_duration(), 2);
    EXPECT_EQ(w2->get_processing_duration(), 1);

    EXPECT_EQ(w1->get_queue()->getQueueType(), PackageQueueType::Fifo);
    EXPECT_EQ(w2->get_queue()->getQueueType(), PackageQueueType::Lifo);

    /* ===== STOREHOUSE ===== */
    auto s1 = factory.find_storehouse_by_id(1);
    ASSERT_NE(s1, factory.storehouse_cend());

    /* ===== LINKS: RAMPS ===== */
    const auto& r1_prefs = r1->get_receiver_preferences();
    ASSERT_EQ(r1_prefs.size(), 1);
    EXPECT_EQ(r1_prefs.begin()->first->get_id(), 1); // worker-1

    const auto& r2_prefs = r2->get_receiver_preferences();
    ASSERT_EQ(r2_prefs.size(), 2);

    std::set<ElementId> r2_targets;
    for (const auto& [receiver, _] : r2_prefs) {
        r2_targets.insert(receiver->get_id());
    }

    EXPECT_TRUE(r2_targets.count(1)); // worker-1
    EXPECT_TRUE(r2_targets.count(2)); // worker-2

    /* ===== LINKS: WORKER-1 ===== */
    const auto& w1_prefs = w1->get_receiver_preferences();
    ASSERT_EQ(w1_prefs.size(), 2);

    std::set<ElementId> w1_targets;
    for (const auto& [receiver, _] : w1_prefs) {
        w1_targets.insert(receiver->get_id());
    }

    EXPECT_TRUE(w1_targets.count(1)); // self-loop
    EXPECT_TRUE(w1_targets.count(2)); // worker-2

    /* ===== LINKS: WORKER-2 ===== */
    const auto& w2_prefs = w2->get_receiver_preferences();
    ASSERT_EQ(w2_prefs.size(), 1);

    auto receiver = w2_prefs.begin()->first;
    EXPECT_EQ(receiver->get_receiver_type(), ReceiverType::STOREHOUSE);
    EXPECT_EQ(receiver->get_id(), 1);

    /* ===== CONSISTENCY ===== */
    EXPECT_TRUE(factory.is_consistent());
}


TEST(SaveFactoryTest, SaveFactoryStructureToTxt) {
    std::istringstream input(
        "; == LOADING RAMPS ==\n"
        "\n"
        "LOADING_RAMP id=1 delivery-interval=3\n"
        "LOADING_RAMP id=2 delivery-interval=2\n"
        "\n"
        "; == WORKERS ==\n"
        "\n"
        "WORKER id=1 processing-time=2 queue-type=FIFO\n"
        "WORKER id=2 processing-time=1 queue-type=LIFO\n"
        "\n"
        "; == STOREHOUSES ==\n"
        "\n"
        "STOREHOUSE id=1\n"
        "\n"
        "; == LINKS ==\n"
        "\n"
        "LINK src=ramp-1 dest=worker-1\n"
        "\n"
        "LINK src=ramp-2 dest=worker-1\n"
        "LINK src=ramp-2 dest=worker-2\n"
        "\n"
        "LINK src=worker-1 dest=worker-1\n"
        "LINK src=worker-1 dest=worker-2\n"
        "\n"
        "LINK src=worker-2 dest=store-1\n"
    );

    IO io;
    Factory factory = io.load_factory_structure(input);

    std::ostringstream output;
    io.save_factory_structure(factory, output);

    std::string saved = output.str();

    /* ===== SPRAWDZANIE DEFINICJI ===== */

    EXPECT_NE(saved.find("LOADING_RAMP id=1 delivery-interval=3"), std::string::npos);
    EXPECT_NE(saved.find("LOADING_RAMP id=2 delivery-interval=2"), std::string::npos);

    EXPECT_NE(saved.find("WORKER id=1 processing-time=2 queue-type=FIFO"), std::string::npos);
    EXPECT_NE(saved.find("WORKER id=2 processing-time=1 queue-type=LIFO"), std::string::npos);

    EXPECT_NE(saved.find("STOREHOUSE id=1"), std::string::npos);

    /* ===== SPRAWDZANIE LINKÓW ===== */

    EXPECT_NE(saved.find("LINK src=ramp-1 dest=worker-1"), std::string::npos);

    EXPECT_NE(saved.find("LINK src=ramp-2 dest=worker-1"), std::string::npos);
    EXPECT_NE(saved.find("LINK src=ramp-2 dest=worker-2"), std::string::npos);

    EXPECT_NE(saved.find("LINK src=worker-1 dest=worker-1"), std::string::npos);
    EXPECT_NE(saved.find("LINK src=worker-1 dest=worker-2"), std::string::npos);

    EXPECT_NE(saved.find("LINK src=worker-2 dest=store-1"), std::string::npos);
}

template<typename T>
std::shared_ptr<T> make_obs_ptr(T* ptr) {
    return std::shared_ptr<T>(ptr, [](T*) {});
}

TEST(FactoryConsistencyTest, EmptyFactoryIsConsistent) {
    Factory f;
    EXPECT_TRUE(f.is_consistent());
}

TEST(FactoryConsistencyTest, RampWithNoReceiverIsInconsistent) {
    Factory f;
    f.add_ramp(Ramp(1, 1));
    EXPECT_FALSE(f.is_consistent());
}

TEST(FactoryConsistencyTest, SimpleValidPathIsConsistent) {
    Factory f;
    f.add_ramp(Ramp(1, 1));
    f.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    f.add_storehouse(Storehouse(1));

    f.find_ramp_by_id(1)->add_receiver(make_obs_ptr<IPackageReceiver>(&(*f.find_worker_by_id(1))));
    f.find_worker_by_id(1)->add_receiver(make_obs_ptr<IPackageReceiver>(&(*f.find_storehouse_by_id(1))));

    EXPECT_TRUE(f.is_consistent());
}

TEST(FactoryConsistencyTest, WorkerWithNoReceiverIsInconsistent) {
    Factory f;
    f.add_ramp(Ramp(1, 1));
    f.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    
    // Rampa -> Worker, ale Worker -> (nic)
    f.find_ramp_by_id(1)->add_receiver(make_obs_ptr<IPackageReceiver>(&(*f.find_worker_by_id(1))));

    EXPECT_FALSE(f.is_consistent());
}

// --- TESTY MODYFIKACJI STRUKTURY ---

TEST(FactoryModificationTest, RemovingStorehouseBreaksConsistency) {
    Factory f;
    f.add_ramp(Ramp(1, 1));
    f.add_storehouse(Storehouse(1));
    f.find_ramp_by_id(1)->add_receiver(make_obs_ptr<IPackageReceiver>(&(*f.find_storehouse_by_id(1))));

    ASSERT_TRUE(f.is_consistent());

    f.remove_storehouse(1);
    // Po usunięciu magazynu rampa traci odbiorcę (logika w Factory::remove_receiver)
    EXPECT_FALSE(f.is_consistent());
}

// --- TESTY IO (Błędy i przypadki brzegowe) ---

TEST(IOTest, ParseLineThrowsOnUnknownType) {
    IO io;
    EXPECT_THROW(io.parse_line("UNKNOWN_TYPE id=1"), std::runtime_error);
}

TEST(IOTest, ParseLineThrowsOnInvalidParameter) {
    IO io;
    // Brak '=' w parametrze
    EXPECT_THROW(io.parse_line("LOADING_RAMP id1"), std::runtime_error);
}

TEST(IOTest, LoadFactoryStructureIgnoresComments) {
    std::istringstream input(
        "; To jest komentarz\n"
        "# To też jest komentarz\n"
        "STOREHOUSE id=1\n"
    );
    IO io;
    Factory f = io.load_factory_structure(input);
    
    EXPECT_NE(f.find_storehouse_by_id(1), f.storehouse_cend());
}

TEST(IOTest, SaveEmptyFactory) {
    Factory f;
    IO io;
    std::ostringstream output;
    io.save_factory_structure(f, output);
    
    // Powinno zwrócić pusty string lub tylko puste linie
    std::string result = output.str();
    // Usuwamy białe znaki do sprawdzenia
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    EXPECT_TRUE(result.empty());
}

// --- TESTY KOLEKCJI (NodeCollection) ---

TEST(NodeCollectionTest, IterationTest) {
    NodeCollection<Storehouse> col;
    col.add(Storehouse(1));
    col.add(Storehouse(2));
    col.add(Storehouse(3));

    size_t count = 0;
    std::vector<ElementId> ids;
    for (const auto& node : col) {
        count++;
        ids.push_back(node.get_id());
    }

    EXPECT_EQ(count, 3);
    EXPECT_EQ(ids[0], 1);
    EXPECT_EQ(ids[2], 3);
}

// --- TESTY SYMULACJI (Krótkie kroki) ---

TEST(FactorySimulationTest, FullStepSimulation) {
    Factory f;
    f.add_ramp(Ramp(1, 1));
    f.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::Fifo)));
    f.add_storehouse(Storehouse(1));

    f.find_ramp_by_id(1)->add_receiver(make_obs_ptr<IPackageReceiver>(&(*f.find_worker_by_id(1))));
    f.find_worker_by_id(1)->add_receiver(make_obs_ptr<IPackageReceiver>(&(*f.find_storehouse_by_id(1))));

    f.do_deliveries(1); 
    
    f.do_deliveries(2); 
    
    f.do_package_passing(); 
    
    auto worker_it = f.find_worker_by_id(1);
    EXPECT_FALSE(worker_it->get_queue()->empty());

    f.do_work(2);
    f.do_work(3); 
    f.do_package_passing();

    
    auto store_it = f.find_storehouse_by_id(1);
    EXPECT_FALSE(store_it->get_queue()->empty());
}