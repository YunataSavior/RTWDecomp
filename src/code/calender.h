// The campaign's dates (code\calender.cpp).
#ifndef CALENDER_H
#define CALENDER_H

// A date of the campaign: a year and a season.
struct Date {
    Date() { m_year = 0; m_season = 2; }
    Date(int year, int season) { m_year = year; m_season = season; }
    int season() const;                         // 0x0043C687: 2 in summer, 0 in winter (a guess)
    void unk_0043C59F();                        // 0x0043C59F: the next season (a guess)
    Date& next_turn();                          // 0x0043BEFE: summer to winter, or winter to the next year's summer
                                                //   (skipping year 0)
    int turns_since(const Date& earlier) const; // 0x0043C046
    Date& previous_turn();                      // 0x0043BF44
    bool operator==(const Date& other) const;   // 0x0043BDDD
    bool operator!=(const Date& other) const;   // 0x0043BEA4
    void add_turns(int turns);                  // 0x0043BF77: back for a negative count
    bool operator>=(const Date& other) const;   // 0x0043BEE0

    int m_year;
    int m_season;
};

#endif
