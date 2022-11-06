#include "database.h"
#include <fcntl.h>

TEST(database, CreateDatabase)
{
    Database::createDatabase(root_path + "/database1");
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}