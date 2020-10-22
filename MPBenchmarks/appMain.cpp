// (c) EB Nov 2009

#include "Config.h"

#include "MemoryTasks.h"
#include "ArithmeticTasks.h"

#if CONFIG_USE_QT
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QtGui/QTextEdit>
#include <QtGui/QClipboard>
#endif

#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <string.h>
#include <sstream>


bool isGPU, isCPU, infoOnly, forcePlatform, forceDevice;
int  specifiedPlatform, specifiedDevice;

struct TestResult
{
  std::string title;
  std::vector<int> mbps; // MB/s by logSize
  void update(int ls,int x) { if (x > mbps[ls]) mbps[ls] = x; } // keep max
};

static const char *helpStr =
    "\n benchmark options [OPTIONS]"
    "\n"
    "\n OPTIONS:"
    "\n  -p, --platform num          choose platform (num starts with 0)"
    "\n  -d, --device num            choose device   (num starts with 0)"
    "\n --cpu - run only CPU tests"
    "\n --gpu - run only GPU tests"
    "\n  --all-tests                 run all above tests [default]"
    "\n --info                       only show info about device"
    "\n  -h, --help                  display help message"
    "\n";
int parseArgs(int argc, char **argv);

int parseArgs(int argc, char **argv)
{
  bool forcedTests = false;
  

  for (int i = 1; i < argc; i++)
  {
    if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
    {
      printf(helpStr);
      //log->print(NEWLINE);
      exit(0);
    }
    else if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--version") == 0))
    {
      //stringstream versionStr;
      //string versionStr << "Benchmark version: " << "1.01";

      //log->print(versionStr.str().c_str());
      printf("Version 1.01\n");
      exit(0);
    }
    else if ((strcmp(argv[i], "-p") == 0) || (strcmp(argv[i], "--platform") == 0))
    {
      if ((i + 1) < argc)
      {
        forcePlatform = true;
        specifiedPlatform = strtoul(argv[i + 1], NULL, 0);
        i++;
      }
    }
    else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--device") == 0))
    {
      if ((i + 1) < argc)
      {
        forceDevice = true;
        specifiedDevice = strtoul(argv[i + 1], NULL, 0);
        i++;
      }
    }
    else if ((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--info") == 0))
    {
        infoOnly = true;
    }
    else if (strcmp(argv[i], "--cpu") == 0)
      {
        isCPU = true;
      }
      else if (strcmp(argv[i], "--gpu") == 0)
      {
        isGPU = true;
      }
    else if (strcmp(argv[i], "--all-tests") == 0)
    {
      isCPU = isGPU = true;
    }
    else
    {
      printf(helpStr);
      //log->print(NEWLINE);
      exit(-1);
    }
  }

  // Allocate logger after parsing
  //log = new logger(enableXml, xmlFileName);
  return 0;
}

void print(FILE * f,const TestResult & r,int minLogSize,int maxLogSize)
{
  fprintf(f,"%-25s",r.title.c_str());
  for (int i=minLogSize;i<=maxLogSize;i++)
    {
      int x = r.mbps[i];
      fprintf(f,"%8d",x);
    }
  fprintf(f,"\n");
}

void print(Logger * log,const TestResult & r,int minLogSize,int maxLogSize)
{
  char aux[200];
  std::string s;
  _snprintf(aux,200,"%-25s",r.title.c_str()); s.append(aux);
  for (int i=minLogSize;i<=maxLogSize;i++)
    {
      int x = r.mbps[i];
      _snprintf(aux,200,"%8d",x); s.append(aux);
    }
  log->append(s);
}

int main(int argc,char ** argv)
{
#if CONFIG_USE_QT
  // Qt windows
  QApplication app(argc,argv);
  QMainWindow * w = new QMainWindow();
  QTextEdit * te = new QTextEdit();
  te->setReadOnly(true);
  w->setCentralWidget(te);
  w->setMinimumSize(QSize(800,600));
  w->show();
  app.processEvents();
  Logger * log = new QtLogger(te);
#else
  // Command line
  Logger * log = new Logger();
#endif

parseArgs(argc, argv);


#if 1
  // Display device info
  cl::Context * c = cl::Context::create();
  if (c != 0)
    {
     // c->getAllDeviceInfo(0,s)
      std::string s;
      if (c->getAllDeviceInfo(specifiedDevice,s)) log->append(s.c_str());
      delete c;
      c = 0;
    }
    
    if (infoOnly)
        exit(0);

#endif

  int minLogSize = 10;
  int maxLogSize = 29;
  std::vector<TestResult> allTests;


if  (isCPU) {
  // Run all CPU tests
  for (int task=0;task<100;task++)
    {
      // Select tests here
      // if (task != 30) continue;
      // continue;

      CPUTask * g = 0;
      // Default values
      std::string title;
      int minNThreads = 1;
      int maxNThreads = 32;

      switch (task)
	{
	case 0:
	  title.assign("CPU copyN");
	  g = new CopyCPUTask();
	  break;
	case 1:
	  title.assign("CPU zeroN");
	  g = new ZeroCPUTask();
	  break;

	case 10:
	  title.assign("CPU addN v1 16-bit");
	  g = new AddNCPUTask(ADDN_V1,16);
 	  break;
	case 11:
	  title.assign("CPU addN v1 32-bit");
	  g = new AddNCPUTask(ADDN_V1,32);
 	  break;
	case 12:
	  title.assign("CPU addN v1 64-bit");
	  g = new AddNCPUTask(ADDN_V1,64);
	  break;

	case 20:
	  title.assign("CPU addN v2 16-bit");
	  g = new AddNCPUTask(ADDN_V2,16);
	  break;
	case 21:
	  title.assign("CPU addN v2 32-bit");
	  g = new AddNCPUTask(ADDN_V2,32);
	  break;
	case 22:
	  title.assign("CPU addN v2 64-bit");
	  g = new AddNCPUTask(ADDN_V2,64);
	  break;

#ifndef Linux
	case 30:
	  title.assign("CPU addN v3 32-bit");
	  g = new AddNCPUTask(ADDN_V3,32);
	  break;
#endif
	}
      if (g == 0) continue; // nothing to do

      TestResult tr;
      tr.title.assign(title);
      tr.mbps.resize(1+maxLogSize,0);
      log->appendTitle(title.c_str());
      for (int nt=minNThreads;nt<=maxNThreads;nt<<=1)
	for (int ls=minLogSize;ls<=maxLogSize;ls++)
	  {
	    size_t sz = 1<<ls;
	    double mbps = g->run(nt,sz);
	    char aux[200];
	    std::string usz;
	    getUserSize(ls,usz);
	    _snprintf(aux,200,"NT=%3d  SZ=%7s   %6.0f MB/s",nt,usz.c_str(),mbps);
	    log->append(aux);
	    tr.update(ls,mbps);
	  }

      print(stdout,tr,minLogSize,maxLogSize);
#if CONFIG_USE_QT
      print(log,tr,minLogSize,maxLogSize);
#endif
      allTests.push_back(tr);
      delete g;
    }

}

if (isGPU) {
  // Run all GPU tests
  for (int task=0;task<100;task++)
    {
      // Select tests here
      //continue;
      //if (task != 30) continue;

      GPUTask * g = 0;
      // Default values
      std::string title;
      int minWorkgroupSize = 16;
      int maxWorkgroupSize = 512;

      switch (task)
	{
	case 0:
	  title.assign("Host to device copy");
	  g = new CopyGPUTask(CopyGPUTask::HOST_TO_DEVICE_COPY,log,specifiedDevice /*, forcePlatform, forceDevice, specifiedPlatform, specifiedDevice */);
	  minWorkgroupSize = maxWorkgroupSize;
	  break;
	case 1:
	  title.assign("Device to host copy");
	  g = new CopyGPUTask(CopyGPUTask::DEVICE_TO_HOST_COPY,log,specifiedDevice /*, forcePlatform, forceDevice, specifiedPlatform, specifiedDevice */);
	  minWorkgroupSize = maxWorkgroupSize;
	  break;
	case 2:
	  title.assign("copyN");
	  g = new CopyGPUTask(CopyGPUTask::DEVICE_TO_DEVICE_COPY,log,specifiedDevice /*, forcePlatform, forceDevice, specifiedPlatform, specifiedDevice*/);
	  minWorkgroupSize = maxWorkgroupSize;
	  break;
	case 3:
	  title.assign("zeroN 32-bit");
	  g = new ZeroGPUTask(32,log,specifiedDevice /*, forcePlatform, forceDevice, specifiedPlatform, specifiedDevice */);
	  break;
	case 4:
	  title.assign("zeroN 64-bit");
	  g = new ZeroGPUTask(64,log,specifiedDevice /*, forcePlatform, forceDevice, specifiedPlatform, specifiedDevice */);
	  break;

	case 10:
	  title.assign("AddN v1 16-bit");
	  g = new AddNGPUTask(ADDN_V1,16,log,specifiedDevice /*, forcePlatform, forceDevice, specifiedPlatform, specifiedDevice*/);
	  break;
	case 11:
	  title.assign("AddN v1 32-bit");
	  g = new AddNGPUTask(ADDN_V1,32,log,specifiedDevice /*, forcePlatform, forceDevice, specifiedPlatform, specifiedDevice*/);
	  break;
	case 12:
	  title.assign("AddN v1 64-bit");
	  g = new AddNGPUTask(ADDN_V1,64,log,specifiedDevice /* ,forcePlatform, forceDevice, specifiedPlatform, specifiedDevice */);
	  break;

	case 20:
	  title.assign("AddN v2 16-bit");
	  g = new AddNGPUTask(ADDN_V2,16,log,specifiedDevice /* ,forcePlatform, forceDevice, specifiedPlatform, specifiedDevice */);
	  break;
	case 21:
	  title.assign("AddN v2 32-bit");
	  g = new AddNGPUTask(ADDN_V2,32,log,specifiedDevice /* ,forcePlatform, forceDevice, specifiedPlatform, specifiedDevice */);
	  break;
	case 22:
	  title.assign("AddN v2 64-bit");
	  g = new AddNGPUTask(ADDN_V2,64,log,specifiedDevice /* ,forcePlatform, forceDevice, specifiedPlatform, specifiedDevice */);
	  break;

	case 40:
	  title.assign("Mul1 v1 32-bit");
	  g = new Mul1GPUTask(MUL1_V1,1,log,specifiedDevice /* ,forcePlatform, forceDevice, specifiedPlatform, specifiedDevice */);
	  break;

	}
      if (g == 0) continue; // nothing to do

      TestResult tr;
      tr.title.assign(title);
      tr.mbps.resize(1+maxLogSize,0);
      log->appendTitle(title.c_str());
      for (int wg=minWorkgroupSize;wg<=maxWorkgroupSize;wg<<=1)
	{
	  char aux[1000];
	  _snprintf(aux,1000,"%s - workgroup_size=%d",title.c_str(),wg);
	  log->append(aux);
	  for (int ls=minLogSize;ls<=maxLogSize;ls++)
	    {
	      size_t sz = 1<<ls;
	      double mbps = g->run(wg,sz);
	      std::string usz;
	      getUserSize(ls,usz);
	      _snprintf(aux,1000,"WG=%3d  SZ=%7s   %6.0f MB/s",wg,usz.c_str(),mbps);
	      log->append(aux);
	      tr.update(ls,mbps);
	    }
	}
      
      print(stdout,tr,minLogSize,maxLogSize);
#if CONFIG_USE_QT
      print(log,tr,minLogSize,maxLogSize);
#endif
      allTests.push_back(tr);
      delete g;
    }
}

  // Dump all results
  int nt = (int)allTests.size();
  log->appendTitle("RESULTS");
  printf("\n----\n");
  for (int i=0;i<nt;i++)
    {
      TestResult & tr = allTests[i];
      print(stdout,tr,minLogSize,maxLogSize);
#if CONFIG_USE_QT
      print(log,tr,minLogSize,maxLogSize);
#endif
    }

#if CONFIG_USE_QT
  // Enter event loop to terminate
  log->appendTitle("DONE");
  app.exec();
#endif
}
