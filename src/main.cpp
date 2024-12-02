#include <iostream>
#include <thread>
#include <cstdlib>
#include <map>
#include <filesystem>
#include <fstream>
#include "world.h"
#include "recording.h"
#include "server_frontend.h"
#include "cpu_bot.h"

using namespace std::chrono;

std::vector<std::string> find_bin_files(const std::string& folder_path)
{
    std::vector<std::string> result {};

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(folder_path))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".bin")
            {
                result.push_back(entry.path().stem().string());
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return result;
}

int test_programs(int seed, CPUProgram& program1, CPUProgram& program2)
{
    int result = 0;
    long limit = 0;
    std::unique_ptr<World> world = std::make_unique<World>(seed);

    world->enable_recording(program1.get_name() + "-" + program2.get_name() + "-" + std::to_string(seed) + ".txt",
        "King-Of-The-Grid | " + program1.get_name() + " vs " +
        program2.get_name() + " seed " + std::to_string(seed));

    world->get_recording()->get_stdout(0).name = program1.get_name();
    world->get_recording()->get_stdout(1).name = program2.get_name();

    std::unique_ptr<Frontend> frontend = std::make_unique<ServerFrontend>(*world);

    world->add_bot(*frontend, new CPUBot(*frontend, program1, *world, 0, 0, BOT_MAX_ENERGY));
    world->add_bot(*frontend, new CPUBot(*frontend, program2, *world, WORLD_SIZE - 1, WORLD_SIZE - 1, BOT_MAX_ENERGY));

    world->start();

    std::cout << "Created world with seed " << world->get_seed() << std::endl;

    if (world->get_recording())
    {
        world->get_recording()->event("Playing: " + program1.get_name() + " vs " + program2.get_name());
    }

    while (world->is_running())
    {
        world->simulate(*frontend);
        frontend->step();
        if (program1.lost() and program2.lost())
        {
            if (world->get_recording())
            {
                world->get_recording()->event(">>>>>>>>>>>> Game ended with Draw.");
            }
            break;
        }
        else if (program1.lost())
        {
            if (world->get_recording())
            {
                world->get_recording()->event(">>>>>>>>>>>> Program " + program2.get_name() + " (2) won.");
            }
            result = 2;
            break;
        }
        else if (program2.lost())
        {
            if (world->get_recording())
            {
                world->get_recording()->event(">>>>>>>>>>>> Program " + program1.get_name() + " (1) won.");
            }
            result = 1;
            break;
        }

        limit++;
        if (limit > ITERATION_LIMIT)
        {
            std::cout << "Reached limit of " << ITERATION_LIMIT << " iterations. " << std::endl;
            if (world->get_recording())
            {
                world->get_recording()->event(std::string("Reached limit of ") + std::to_string(ITERATION_LIMIT) + " iterations.");
            }
            result = 0;
            break;
        }
    }

    std::cout << "Stopped world " << std::endl;

    world.reset();
    frontend.reset();

    return result;
}

int main(int argc, char** argv)
{
    srand(time(0));

    if (argc <= 1)
    {
        std::map<std::string, CPUProgram> programs {};

        for (auto program: find_bin_files("."))
        {
            programs.emplace(std::piecewise_construct,
                std::forward_as_tuple(program),
                std::forward_as_tuple(program, program + ".bin", programs.empty()));
        }

        if (programs.empty())
        {
            throw std::runtime_error("No programs found.");
        }

        if (programs.size() < 2)
        {
            throw std::runtime_error("Server runtime needs at least two runtimes.");
        }

        srand(time(0));

        std::map<std::string, std::vector<std::string>> wins {};

        // Nested loop to test each program against all subsequent programs
        for (auto it1 = programs.begin(); it1 != programs.end(); ++it1)
        {
            for (auto it2 = std::next(it1); it2 != programs.end(); ++it2)
            {
                for (int i = 1; i <= 3; i++)
                {
                    int seed = rand();

                    std::cout << "-------------------" << std::endl;
                    std::cout << " TAKE #" << i << " testing programs " << it1->first << " and " << it2->first << " on seed " << seed << "." << std::endl;
                    std::cout << "-------------------" << std::endl;

                    {
                        int result1 = test_programs(seed, it1->second, it2->second);

                        std::cout << "-------------------" << std::endl;
                        switch (result1)
                        {
                            case 1:
                            {
                                it1->second.bump_score();
                                wins[it1->first].push_back("\"" + it1->first + " " + it2->first + " " + std::to_string(seed) + "\"");
                                std::cout << "A: Program " << it1->first << " (1) won." << std::endl;
                                break;
                            }
                            case 2:
                            {
                                it2->second.bump_score();
                                wins[it2->first].push_back("\"" + it1->first + " " + it2->first + " " + std::to_string(seed) + "\"");
                                std::cout << "A: Program " << it2->first << " (2) won." << std::endl;
                                break;
                            }
                            case 0:
                            default:
                            {
                                std::cout << "A: Draw." << std::endl;
                                break;
                            }
                        }

                        std::cout << "-------------------" << std::endl;
                    }

                    // swap places
                    {
                        int result2 = test_programs(seed, it2->second, it1->second);

                        switch (result2)
                        {
                            case 1:
                            {
                                it2->second.bump_score();
                                wins[it2->first].push_back("\"" + it2->first + " " + it1->first + " " + std::to_string(seed) + "\"");
                                std::cout << "B: Program " << it2->first << " (2) won." << std::endl;
                                break;
                            }
                            case 2:
                            {
                                it1->second.bump_score();
                                wins[it1->first].push_back("\"" + it2->first + " " + it1->first + " " + std::to_string(seed) + "\"");
                                std::cout << "B: Program " << it1->first << " (1) won." << std::endl;
                                break;
                            }
                            case 0:
                            default:
                            {
                                std::cout << "B: Draw." << std::endl;
                                break;
                            }
                        }
                    }
                }
            }
        }

        std::cout << "-------------------" << std::endl;
        std::cout << "OUTCOME" << std::endl;
        std::cout << "-------------------" << std::endl;

        {
            std::stringstream outcome;

            int index = 1;

            for (auto p: programs)
            {
                outcome << index << ". " << p.first << " | " << p.second.get_score() << " win(s) | ";
                index++;

                bool first = true;
                for (const auto& win: wins[p.first])
                {
                    if (first)
                    {
                        first = false;
                        outcome << win;
                    }
                    else
                    {
                        outcome << ", " << win;
                    }
                }
                outcome << std::endl;
            }

            std::cout << outcome.str();

            {
                std::ofstream fo("outcome.txt");
                fo << outcome.str();
            }
        }

        std::cout << "-------------------" << std::endl;

        return 0;
    }
    else
    {
        std::string program1_name = argv[1];

        std::string program2_name;
        if (argc > 2)
        {
            program2_name = argv[2];
        }
        else
        {
            std::cout << "Only one program specified: using against itself." << std::endl;
            program2_name = program1_name;
        }

        int seed;

        if (argc > 3)
        {
            seed = std::atoi(argv[3]);
        }
        else
        {
            srand(time(0));
            seed = rand();

            std::cout << "Seed not provided. Using random: " << seed << std::endl;
        }

        CPUProgram first_program(program1_name, program1_name + ".bin", true);
        CPUProgram second_program(program2_name, program2_name + ".bin", false);

        int result = test_programs(seed, first_program, second_program);

        switch (result)
        {
            case 1:
            {
                std::cout << "Program " << first_program.get_name() << " (1) won." << std::endl;
                break;
            }
            case 2:
            {
                std::cout << "Program " << second_program.get_name() << " (2) won." << std::endl;
                break;
            }
            case 0:
            default:
            {
                std::cout << "Draw." << std::endl;
                break;
            }
        }

        return result;
    }
}
