#include	<stdlib.h>
#include  <TestGame.h>


/*!
 * Main function. Creates a TestGame objects and calls testGame.Start().
 * @param argc does nothing; it is left here for convention or arguments in the future.
 * @param argv does nothing; it is left here for convention or arguments in the future.
 * @return returns 0 if the program exited without detected issues.
 */
int main (int argc, char* argv[])
{
    TestGame testGame;
    testGame.Start();
    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
