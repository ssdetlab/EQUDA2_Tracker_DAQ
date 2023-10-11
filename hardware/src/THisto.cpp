#include "THisto.h"
#include <iostream>

THisto::THisto()
{
  m_ndim      = 0;
  m_name      = "";
  m_title     = "";
  m_dim[0]    = 0;
  m_dim[1]    = 0;
  m_lim[0][0] = 0;
  m_lim[0][1] = 0;
  m_lim[1][0] = 0;
  m_lim[1][1] = 0;
  m_histo     = 0;
  m_trash     = 0;
  m_size      = 8;
}

THisto::THisto(std::string name, std::string title, unsigned int nbin, double xmin, double xmax)
{
  m_ndim      = 1;
  m_name      = name;
  m_title     = title;
  m_dim[0]    = nbin;
  m_dim[1]    = 1;
  m_lim[0][0] = xmin;
  m_lim[0][1] = xmax;
  m_lim[1][0] = 0;
  m_lim[1][1] = 0;
  m_histo     = new void *[1];
  m_histo[0]  = (void *)new double[m_dim[0]];
  for (unsigned int i = 0; i < m_dim[0]; i++)
    ((double **)m_histo)[0][i] = 0;
  m_trash = 0;
  m_size  = 8;
}

THisto::THisto(std::string name, std::string title, unsigned int nbin1, double xmin1, double xmax1,
               unsigned int nbin2, double xmin2, double xmax2)
{
  m_ndim      = 2;
  m_name      = name;
  m_title     = title;
  m_dim[0]    = nbin1;
  m_dim[1]    = nbin2;
  m_lim[0][0] = xmin1;
  m_lim[0][1] = xmax1;
  m_lim[1][0] = xmin2;
  m_lim[1][1] = xmax2;
  m_histo     = new void *[m_dim[1]];
  for (unsigned int j = 0; j < m_dim[1]; j++) {
    m_histo[j] = (void *)new double[m_dim[0]];
    for (unsigned int i = 0; i < m_dim[0]; i++)
      ((double **)m_histo)[j][i] = 0;
  }
  m_trash = 0;
  m_size  = 8;
}

THisto::THisto(std::string name, std::string title, unsigned int size, unsigned int nbin1,
               double xmin1, double xmax1, unsigned int nbin2, double xmin2, double xmax2)
{
  m_ndim = 2;
  if (size != 1 && size != 2 && size != 4) size = 8;
  m_size      = size;
  m_name      = name;
  m_title     = title;
  m_dim[0]    = nbin1;
  m_dim[1]    = nbin2;
  m_lim[0][0] = xmin1;
  m_lim[0][1] = xmax1;
  m_lim[1][0] = xmin2;
  m_lim[1][1] = xmax2;
  m_histo     = new void *[m_dim[1]];
  if (m_size == 1) {
    for (unsigned int j = 0; j < m_dim[1]; j++) {
      m_histo[j] = (void *)new unsigned char[m_dim[0]];
      for (unsigned int i = 0; i < m_dim[0]; i++)
        ((unsigned char **)m_histo)[j][i] = 0;
    }
  }
  else if (m_size == 2) {
    for (unsigned int j = 0; j < m_dim[1]; j++) {
      m_histo[j] = (void *)new unsigned short int[m_dim[0]];
      for (unsigned int i = 0; i < m_dim[0]; i++)
        ((unsigned short int **)m_histo)[j][i] = 0;
    }
  }
  else if (m_size == 4) {
    for (unsigned int j = 0; j < m_dim[1]; j++) {
      m_histo[j] = (void *)new float[m_dim[0]];
      for (unsigned int i = 0; i < m_dim[0]; i++)
        ((float **)m_histo)[j][i] = 0;
    }
  }
  else {
    for (unsigned int j = 0; j < m_dim[1]; j++) {
      m_histo[j] = (void *)new double[m_dim[0]];
      for (unsigned int i = 0; i < m_dim[0]; i++)
        ((double **)m_histo)[j][i] = 0;
    }
  }
  m_trash = 0;
}

THisto::THisto(const THisto &h)
{
  m_ndim      = h.m_ndim;
  m_name      = h.m_name;
  m_title     = h.m_title;
  m_dim[0]    = h.m_dim[0];
  m_dim[1]    = h.m_dim[1];
  m_lim[0][0] = h.m_lim[0][0];
  m_lim[0][1] = h.m_lim[0][1];
  m_lim[1][0] = h.m_lim[1][0];
  m_lim[1][1] = h.m_lim[1][1];
  m_size      = h.m_size;
  m_histo     = new void *[m_dim[1]];
  if (m_size == 1) {
    for (unsigned int j = 0; j < m_dim[1]; j++) {
      m_histo[j] = (void *)new unsigned char[m_dim[0]];
      for (unsigned int i = 0; i < m_dim[0]; i++)
        ((unsigned char **)m_histo)[j][i] = ((unsigned char **)h.m_histo)[j][i];
    }
  }
  else if (m_size == 2) {
    for (unsigned int j = 0; j < m_dim[1]; j++) {
      m_histo[j] = (void *)new unsigned short int[m_dim[0]];
      for (unsigned int i = 0; i < m_dim[0]; i++)
        ((unsigned short int **)m_histo)[j][i] = ((unsigned short int **)h.m_histo)[j][i];
    }
  }
  else if (m_size == 4) {
    for (unsigned int j = 0; j < m_dim[1]; j++) {
      m_histo[j] = (void *)new float[m_dim[0]];
      for (unsigned int i = 0; i < m_dim[0]; i++)
        ((float **)m_histo)[j][i] = ((float **)h.m_histo)[j][i];
    }
  }
  else {
    for (unsigned int j = 0; j < m_dim[1]; j++) {
      m_histo[j] = (void *)new double[m_dim[0]];
      for (unsigned int i = 0; i < m_dim[0]; i++)
        ((double **)m_histo)[j][i] = ((double **)h.m_histo)[j][i];
    }
  }
  m_trash = 0;
}

THisto::THisto(THisto &&h)
    : m_ndim(h.m_ndim), m_name(h.m_name), m_title(h.m_title), m_dim(h.m_dim), m_lim(h.m_lim),
      m_histo(h.m_histo), m_size(h.m_size), m_trash(0)
{
  h.m_dim   = {0, 0};
  h.m_lim   = {{{0, 0}, {0, 0}}};
  h.m_histo = nullptr;
}

THisto &THisto::operator=(THisto &&h)
{
  if (&h != this) {
    std::swap(m_ndim, h.m_ndim);
    std::swap(m_name, h.m_name);
    std::swap(m_title, h.m_title);
    std::swap(m_dim, h.m_dim);
    std::swap(m_lim, h.m_lim);
    std::swap(m_histo, h.m_histo);
    std::swap(m_size, h.m_size);
    std::swap(m_trash, h.m_trash);
  }
  return *this;
}

THisto::~THisto()
{
  for (unsigned int j = 0; j < m_dim[1]; j++) {
    if (m_size == 1) delete[]((unsigned char **)m_histo)[j];
    if (m_size == 2) delete[]((unsigned short int **)m_histo)[j];
    if (m_size == 4) delete[]((float **)m_histo)[j];
    if (m_size == 8) delete[](double *)(m_histo[j]);
  }
  delete[] m_histo;
}

THisto &THisto::operator=(const THisto &h)
{
  if (&h == this) {
    return *this;
  }
  else {
    unsigned int j;
    for (j = 0; j < m_dim[1]; j++) {
      if (m_size == 1) delete[]((unsigned char **)m_histo)[j];
      if (m_size == 2) delete[]((unsigned short int **)m_histo)[j];
      if (m_size == 4) delete[]((float **)m_histo)[j];
      if (m_size == 8) delete[]((double **)m_histo)[j];
    }
    delete[] m_histo;
    m_ndim      = h.m_ndim;
    m_name      = h.m_name;
    m_title     = h.m_title;
    m_dim[0]    = h.m_dim[0];
    m_dim[1]    = h.m_dim[1];
    m_lim[0][0] = h.m_lim[0][0];
    m_lim[0][1] = h.m_lim[0][1];
    m_lim[1][0] = h.m_lim[1][0];
    m_lim[1][1] = h.m_lim[1][1];
    m_size      = h.m_size;
    m_histo     = new void *[m_dim[1]];
    if (m_size == 1) {
      for (unsigned int j = 0; j < m_dim[1]; j++) {
        m_histo[j] = (void *)new unsigned char[m_dim[0]];
        for (unsigned int i = 0; i < m_dim[0]; i++)
          ((unsigned char **)m_histo)[j][i] = ((unsigned char **)h.m_histo)[j][i];
      }
    }
    else if (m_size == 2) {
      for (unsigned int j = 0; j < m_dim[1]; j++) {
        m_histo[j] = (void *)new unsigned short int[m_dim[0]];
        for (unsigned int i = 0; i < m_dim[0]; i++)
          ((unsigned short int **)m_histo)[j][i] = ((unsigned short int **)h.m_histo)[j][i];
      }
    }
    else if (m_size == 4) {
      for (unsigned int j = 0; j < m_dim[1]; j++) {
        m_histo[j] = (void *)new float[m_dim[0]];
        for (unsigned int i = 0; i < m_dim[0]; i++)
          ((float **)m_histo)[j][i] = ((float **)h.m_histo)[j][i];
      }
    }
    else {
      for (unsigned int j = 0; j < m_dim[1]; j++) {
        m_histo[j] = (void *)new double[m_dim[0]];
        for (unsigned int i = 0; i < m_dim[0]; i++)
          ((double **)m_histo)[j][i] = ((double **)h.m_histo)[j][i];
      }
    }
    m_trash = 0;
    return *this;
  }
}

double THisto::operator()(unsigned int i) const
{
  if (i < m_dim[0]) {
    if (m_size == 1) return (double)(((unsigned char **)m_histo)[0][i]);
    if (m_size == 2) return (double)(((unsigned short int **)m_histo)[0][i]);
    if (m_size == 4) return (double)(((float **)m_histo)[0][i]);
    if (m_size == 8) return (double)(((double **)m_histo)[0][i]);
  }
  return m_trash;
}

double THisto::operator()(unsigned int i, unsigned int j) const
{
  if (i < m_dim[0] && j < m_dim[1]) {
    if (m_size == 1) return (double)(((unsigned char **)m_histo)[j][i]);
    if (m_size == 2) return (double)(((unsigned short int **)m_histo)[j][i]);
    if (m_size == 4) return (double)(((float **)m_histo)[j][i]);
    if (m_size == 8) return (double)(((double **)m_histo)[j][i]);
  }
  return m_trash;
}

void THisto::Set(unsigned int i, double val)
{
  if (i < m_dim[0]) {
    if (m_size == 1) ((unsigned char **)m_histo)[0][i] = (unsigned char)val;
    if (m_size == 2) ((unsigned short int **)m_histo)[0][i] = (unsigned short int)val;
    if (m_size == 4) ((float **)m_histo)[0][i] = (float)val;
    if (m_size == 8) ((double **)m_histo)[0][i] = val;
  }
}

void THisto::Set(unsigned int i, unsigned int j, double val)
{
  if (i < m_dim[0] && j < m_dim[1]) {
    if (m_size == 1) ((unsigned char **)m_histo)[j][i] = (unsigned char)val;
    if (m_size == 2) ((unsigned short int **)m_histo)[j][i] = (unsigned short int)val;
    if (m_size == 4) ((float **)m_histo)[j][i] = (float)val;
    if (m_size == 8) ((double **)m_histo)[j][i] = val;
  }
}

void THisto::Incr(unsigned int i)
{
  // if (i>m_lim[0][0]-1 && i<m_lim[0][1]+1) {
  if (i < m_dim[0]) {
    if (m_size == 1) ((unsigned char **)m_histo)[0][i]++;
    if (m_size == 2) ((unsigned short int **)m_histo)[0][i]++;
    if (m_size == 4) ((float **)m_histo)[0][i]++;
    if (m_size == 8) ((double **)m_histo)[0][i]++;
  }
}

void THisto::Incr(unsigned int i, unsigned int j)
{
  // if (i>m_lim[0][0]-1 && i<m_lim[0][1]+1 && j>m_lim[1][0]-1 && j<m_lim[1][1]+1) {
  if (i < m_dim[0] && j < m_dim[1]) {
    // std::cout << i << ", " << j << std::endl;
    // std::cout << ((unsigned short int **)m_histo)[j][i] << std::endl;
    if (m_size == 1) ((unsigned char **)m_histo)[j][i]++;
    if (m_size == 2) ((unsigned short int **)m_histo)[j][i]++;
    if (m_size == 4) ((float **)m_histo)[j][i]++;
    if (m_size == 8) ((double **)m_histo)[j][i]++;
  }
}

void THisto::Clear()
{
  for (unsigned int j = 0; j < m_dim[1]; j++) {
    for (unsigned int i = 0; i < m_dim[0]; i++) {
      if (m_size == 1) ((unsigned char **)m_histo)[j][i] = 0;
      if (m_size == 2) ((unsigned short int **)m_histo)[j][i] = 0;
      if (m_size == 4) ((float **)m_histo)[j][i] = 0;
      if (m_size == 8) ((double **)m_histo)[j][i] = 0;
    }
  }
  m_trash = 0;
}

//================================================================================
//
//                         TScanHisto
//
//================================================================================

TScanHisto::TScanHisto(const TScanHisto &sh)
{
  std::map<int, THisto>::const_iterator it;
  for (it = sh.m_histos.begin(); it != sh.m_histos.end(); ++it) {
    m_histos.insert(*it);
  }
  SetIndex(sh.GetIndex());
}

TScanHisto::~TScanHisto() { m_histos.clear(); }

void TScanHisto::AddHisto(common::TChipIndex index, THisto histo)
{
  int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  m_histos.insert(std::pair<int, THisto>(int_index, histo));
}

void TScanHisto::Clear()
{
  std::map<int, THisto>::iterator it;
  for (it = m_histos.begin(); it != m_histos.end(); ++it) {
    (*it).second.Clear();
  }
  m_index = -1;
}

void TScanHisto::Incr(common::TChipIndex index, unsigned int i, unsigned int j)
{
  int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  try {
    m_histos.at(int_index).Incr(i, j);
  }
  catch (const std::out_of_range &e) {
    std::cerr << "Invalid index: board " << index.boardIndex << ", dataReceiver "
              << index.dataReceiver << ", chipID " << index.chipId << "!" << std::endl;
  }
}

void TScanHisto::Incr(common::TChipIndex index, unsigned int i)
{
  int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  try {
    m_histos.at(int_index).Incr(i);
  }
  catch (const std::out_of_range &e) {
    std::cerr << "Invalid index: board " << index.boardIndex << ", dataReceiver "
              << index.dataReceiver << ", chipID " << index.chipId << "!" << std::endl;
  }
}

void TScanHisto::Set(common::TChipIndex index, unsigned int i, unsigned int j, double val)
{
  int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  try {
    m_histos.at(int_index).Set(i, j, val);
  }
  catch (const std::out_of_range &e) {
    std::cerr << "Invalid index: board " << index.boardIndex << ", dataReceiver "
              << index.dataReceiver << ", chipID " << index.chipId << "!" << std::endl;
  }
}

void TScanHisto::Set(common::TChipIndex index, unsigned int i, double val)
{
  int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  try {
    m_histos.at(int_index).Set(i, val);
  }
  catch (const std::out_of_range &e) {
    std::cerr << "Invalid index: board " << index.boardIndex << ", dataReceiver "
              << index.dataReceiver << ", chipID " << index.chipId << "!" << std::endl;
  }
}

// TODO clean up
double TScanHisto::operator()(common::TChipIndex index, unsigned int i, unsigned int j) const
{
  int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  return (m_histos.at(int_index))(i, j);
}

double TScanHisto::operator()(common::TChipIndex index, unsigned int i) const
{
  int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  return (m_histos.at(int_index))(i);
}

int TScanHisto::GetNBin(common::TChipIndex index, int d)
{
  int result    = -1;
  int int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  try {
    result = m_histos.at(int_index).GetNBin(d);
  }
  catch (const std::out_of_range &e) {
    std::cerr << "Invalid index: board " << index.boardIndex << ", dataReceiver "
              << index.dataReceiver << ", chipID " << index.chipId << "!" << std::endl;
  }
  return result;
}

double TScanHisto::GetMin(common::TChipIndex index, int d)
{
  double result    = -1;
  int    int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  try {
    result = m_histos.at(int_index).GetMin(d);
  }
  catch (const std::out_of_range &e) {
    std::cerr << "Invalid index: board " << index.boardIndex << ", dataReceiver "
              << index.dataReceiver << ", chipID " << index.chipId << "!" << std::endl;
  }
  return result;
}

double TScanHisto::GetMax(common::TChipIndex index, int d)
{
  double result    = -1;
  int    int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  try {
    result = m_histos.at(int_index).GetMax(d);
  }
  catch (const std::out_of_range &e) {
    std::cerr << "Invalid index: board " << index.boardIndex << ", dataReceiver "
              << index.dataReceiver << ", chipID " << index.chipId << "!" << std::endl;
  }
  return result;
}

double TScanHisto::GetStep(common::TChipIndex index, int d)
{
  double result    = -1;
  int    int_index = (index.boardIndex << 8) | (index.dataReceiver << 4) | (index.chipId & 0xf);
  try {
    result = m_histos.at(int_index).GetStep(d);
  }
  catch (const std::out_of_range &e) {
    std::cerr << "Invalid index: board " << index.boardIndex << ", dataReceiver "
              << index.dataReceiver << ", chipID " << index.chipId << "!" << std::endl;
  }
  return result;
}


int TScanHisto::GetChipList(std::vector<common::TChipIndex> &chipList)
{
  chipList.clear();
  for (std::map<int, THisto>::iterator it = m_histos.begin(); it != m_histos.end(); ++it) {
    int                intIndex = it->first;
    common::TChipIndex index;
    index.boardIndex   = (intIndex >> 8);
    index.dataReceiver = (intIndex >> 4) & 0xf;
    index.chipId       = intIndex & 0xf;
    chipList.push_back(index);
  }
  return chipList.size();
}
