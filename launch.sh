#Consider flag for turning off testing, or turning off fresh compile to just launch etc.
#
echo "##########################################################"
echo "#             Begin TSP Solver Solution                  #"
echo "##########################################################"

DEBUG_FLAGS="-fsanitize=address -g -O0 -Wall -Werror"
NO_OPT_FLAGS="-O0 -g -fno-omit-frame-pointer -fno-optimize-sibling-calls"
LIGHT_DBG_FLAGS="-g -O0 -Wall -Werror -fno-optimize-sibling-calls -fno-omit-frame-pointer"
CFLAGS=$LIGHT_DBG_FLAGS
INCLUDE_FLAGS="-Iinclude -Isrc -Isrc/tsp -Isrc/memory"
TEST_ONLY=false

#for arg in "$@"; do
    #if [ "$arg" = "-d" ] || [ "$arg" = "--debug" ]; then
        #echo "[!] Debug mode enabled."
        #CFLAGS=$DEBUG_FLAGS
    #fi  
#done

for arg in "$@"; do 
    case "$arg" in 
        -d|--debug)
            echo "[!] Debug mode enable."
            CFLAGS=$DEBUG_FLAGS
            ;;
        -t|--test-only)
            echo "[!] Running in test-only mode."
            TEST_ONLY=true;;
        -n|--no-optimize)
            echo "[!] Running with no compiler optimization."
            CFLAGS=$DEBUG_FLAGS
            ;;
        *)
            echo "Syntax: launch.sh [flag][flag]"
            echo "Flags --debug, -d, --test-only, -t, --no-optimize, -n (overwrites --debug)"
            ;;
    esac
done

mkdir -p build
mkdir -p bin

echo "[.] Compiling object files..."
clang -std=c99 $CFLAGS -c src/memory/scratch_arena.c -o build/scratch_arena.o $INCLUDE_FLAGS 
clang -std=c99 $CFLAGS -c src/tsp/dist_matrix.c -o build/dist_matrix.o $INCLUDE_FLAGS
clang -std=c99 $CFLAGS -c src/memory/page_arena.c -o build/page_arena.o $INCLUDE_FLAGS
clang -std=c99 $CFLAGS -c src/tsp/trie.c -o build/trie $INCLUDE_FLAGS
#add as needed here:

#add "runner" here:
#TARGET="bin/tsp_solver"
#clang -std=c99 $CFLAGS build/*.o -o $TARGET $INCLUDE_FLAGS

if [ $? -ne 0 ]; then
    echo "[ ] Compilation/Linkage Failed."
    exit 1
fi

echo "[X] Main Compilation Complete"

if [ "$TEST_ONLY" = true ]; then
    echo
    echo "##########################################################"
    echo "#                   Running Unit Tests....               #"
    echo "##########################################################"

    if ! ./test.sh; then
        echo "[ ] Test compilation failed."
        exit 1
    fi

    if ! ./runtests.sh; then
        echo "[ ] Unit tests failed."
        exit 1
    fi

    echo "[X] All tests passed."
    exit 0

fi

echo 
echo "##########################################################"
echo "#                   Running Unit Tests....               #"
echo "##########################################################"

if ! ./test.sh; then
    echo "[ ] Test compilation failed."
    exit 1
fi

if ! ./runtests.sh; then
    echo "[ ] Unit tests failed."
    exit 1
fi

echo "[X] All tests passed."
echo
echo "##########################################################"
echo "#                  Begin TSP Solver Solutions            #"
echo "##########################################################"
echo

./$TARGET
