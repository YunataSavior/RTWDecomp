#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\spy_actions.cpp"
// The spies' actions.
static char THIS_FILE[] = __FILE__;

#include <math.h>

#include "unsorted.h"
#include "character.h"
#include "character_record.h"
#include "spy_actions.h"

float combined_subterfuge(Array<Character*>& characters)
{
    float sum = 0.0f;
    for (unsigned int i = 0; i < characters.size(); i++) {
        int subterfuge = characters[i]->record()->subterfuge() + 1;
        sum += subterfuge * subterfuge;
    }
    if (sum == 0.0f)
        return 1.0f;
    return (float)sqrt(sum) - 1.0f;
}
