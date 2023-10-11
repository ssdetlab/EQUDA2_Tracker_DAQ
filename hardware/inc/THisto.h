#ifndef THISTO_H
#define THISTO_H

#include <map>
#include <string>
#include <vector>

#include "Common.h"

/* typedef struct  */
/* { */
/*   unsigned int boardIndex; */
/*   unsigned int dataReceiver; */
/*   unsigned int chipId; */
/* } TChipIndex; */

class THisto {

private:
  int                                  m_ndim;  // Number of dimensions (1 or 2)
  std::string                          m_name;  // Histogram name
  std::string                          m_title; // Histogram title
  std::array<unsigned int, 2>          m_dim;   // Dimensions
  std::array<std::array<double, 2>, 2> m_lim;   // Limits
  void **                              m_histo; // Histogram
  unsigned int                         m_size;  // Word size
  double                               m_trash; // Trash bin


public:
  THisto(); // Default constructor ("0-Dim histogram")
  THisto(std::string name, std::string title, unsigned int nbin, double xmin,
         double xmax); // Constructor 1D
  THisto(std::string name, std::string title, unsigned int nbin1, double xmin1, double xmax1,
         unsigned int nbin2, double xmin2, double xmax2); // Constructor 2D
  THisto(std::string name, std::string title, unsigned int size, unsigned int nbin1, double xmin1,
         double xmax1, unsigned int nbin2, double xmin2, double xmax2); // Constructor 2D
  THisto(const THisto &h);                                              // Copy constructor
  THisto(THisto &&);                                                    // Move constructor
  ~THisto();                                                            // Destructor
  THisto &operator=(const THisto &h);                                   // Assignment operator
  THisto &operator=(THisto &&h);                                        // Move assignment
  double  operator()(unsigned int i) const;                             // Bin read access 1d
  double  operator()(unsigned int i, unsigned int j) const;             // Bin read access 2d
  void    Set(unsigned int i, double val);                              // Bin write access 1d
  void    Set(unsigned int i, unsigned int j, double val);              // Bin write access 2d
  void    Incr(unsigned int i);
  void    Incr(unsigned int i, unsigned int j);
  void    Clear(); // Reset histo - NO MEMORY DISCARD

  //! Getter methods
  std::string GetName() const { return m_name; };
  std::string GetTitle() const { return m_title; };
  int         GetNDim() const { return m_ndim; };
  int         GetNBin(int d) const
  {
    if (d >= 0 && d <= 1)
      return m_dim[d];
    else
      return 0;
  };
  double GetMin(int d) const
  {
    if (d >= 0 && d <= 1)
      return m_lim[d][0];
    else
      return 0;
  };
  double GetMax(int d) const
  {
    if (d >= 0 && d <= 1)
      return m_lim[d][1];
    else
      return 0;
  };
  double GetStep(int d) const
  {
    if (GetNBin(d) > 1)
      return (GetMax(d) - GetMin(d)) / ((double)GetNBin(d) - 1.);
    else
      return 0;
  }
};

class TScanHisto {
private:
  std::map<int, THisto> m_histos;
  int                   m_index;

public:
  TScanHisto(){};                   // Default constructor;
  TScanHisto(const TScanHisto &sh); // Copy constructor;
  TScanHisto(TScanHisto &&) = default;
  ~TScanHisto();
  TScanHisto &operator=(TScanHisto &&h); // Move assignment
  double      operator()(common::TChipIndex index, unsigned int i,
                    unsigned int j) const; // Bin read access 2d
  double      operator()(common::TChipIndex index, unsigned int i) const;
  void        AddHisto(common::TChipIndex index, THisto histo);
  int         GetSize() { return m_histos.size(); };
  int         GetChipList(std::vector<common::TChipIndex> &chipList);
  void        Clear();
  void        SetIndex(int aIndex) { m_index = aIndex; };
  int         GetIndex() const { return m_index; };
  void        Set(common::TChipIndex index, unsigned int i, double val); // Bin write access 1d
  void        Set(common::TChipIndex index, unsigned int i, unsigned int j,
                  double val); // Bin write access 2d
  void        Incr(common::TChipIndex index, unsigned int i, unsigned int j);
  void        Incr(common::TChipIndex index, unsigned int i);
  std::map<int, THisto> GetHistoMap() { return m_histos; }
  int                   GetNBin(common::TChipIndex index, int d);
  double                GetMin(common::TChipIndex index, int d);
  double                GetMax(common::TChipIndex index, int d);
  double                GetStep(common::TChipIndex index, int d);
};

#endif
