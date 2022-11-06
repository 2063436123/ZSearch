#include <xapian.h>

#include <iostream>
#include <string>

#include <cstdlib> // For exit().
#include <cstring>

using namespace std;

int main(int argc, char **argv)
try
{
    // We require at least two command line arguments.
    if (argc < 2)
    {
        int rc = 1;
        if (argv[1])
        {
            if (strcmp(argv[1], "--version") == 0)
            {
                cout << "simplesearch\n";
                exit(0);
            }
            if (strcmp(argv[1], "--help") == 0)
            {
                rc = 0;
            }
        }
        cout << "Usage: " << argv[0] << " PATH_TO_DATABASE\n";
        exit(rc);
    }

    // Open the database for searching.
    Xapian::Database db(argv[1]);

    // Start an enquire session.
    Xapian::Enquire enquire(db);

    std::string query_str;
    while (true)
    {
        cout << "enter query: ";
        getline(cin, query_str);
        if (!cin)
            break;
        // Combine the rest of the command line arguments with spaces between
        // them, so that simple queries don't have to be quoted at the shell
        // level.
        string query_string(query_str);

        // Parse the query string to produce a Xapian::Query object.
        Xapian::QueryParser qp;
        Xapian::Stem stemmer("english");
        qp.set_stemmer(stemmer);
        qp.set_database(db);
        qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
        Xapian::Query query = qp.parse_query(query_string);
        cout << "Parsed query is: " << query.get_description() << '\n';

        // Find the top 10 results for the query.
        enquire.set_query(query);
        Xapian::MSet matches = enquire.get_mset(0, 10);

        // Display the results.
        cout << matches.get_matches_estimated() << " results found.\n";
        cout << "\tMatches 1-" << matches.size() << ":\n\n";

        for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i)
        {
            cout << '\t' << i.get_rank() + 1 << ": " << i.get_weight() << " docid=" << *i
                 << " [" << i.get_document().get_data() << "]\n\n";
        }
    }
}
catch (const Xapian::Error &e)
{
    cout << e.get_description() << '\n';
    exit(1);
}