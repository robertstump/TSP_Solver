echo
echo "##########################################################"
echo "#                 Compiling All Tests....                #"
echo "##########################################################"

clang -std=c99 -Wall -Werror tests/test_scratch_arena.c build/*.o -o test_lib/scratch_arena_tests -Iinclude -Isrc -Isrc/tsp -Isrc/memory
clang -std=c99 -Wall -Werror tests/test_dist_matrix.c build/*.o -o test_lib/dist_matrix_tests -Iinclude -Isrc -Isrc/tsp -Isrc/memory
clang -std=c99 -Wall -Werror tests/test_page_arena.c build/*.o -o test_lib/page_arena_tests -Iinclude -Isrc -Isrc/tsp -Isrc/memory

if [ $? -eq 0 ]; then
    echo "[X] Tests compilation complete...."
    exit 0

fi

echo "[X] Compilation Failed"

