#include "./FTXUI_utils.h"
#include "Block.hpp"
#include "BlockChain.hpp"
#include "Transaction.hpp"
#include "Utils.hpp"
#include "Wallet.hpp"
#include "ftxui/component/component.hpp" // for Slider, Checkbox, Vertical, Renderer, Button, Input, Menu, Radiobox, Toggle
#include "ftxui/component/component_base.hpp" // for ComponentBase
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp" // for separator, operator|, Element, size, xflex, text, WIDTH, hbox, vbox, EQUAL, border, GREATER_THAN
#include <chrono>
#include <cstdio>
#include <fstream>
#include <ftxui/dom/direction.hpp>
#include <ftxui/screen/color.hpp>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

using namespace ftxui;
std::mutex ThreadLock;
std::string DisplayMain = "";
std::string TerminalDisplay = "";
BlockChain LocalChain;
Wallet LocalWallet;
std::string PrivateKey = "";
Transaction CurrentTransaction;

std::stringstream buffer;
std::streambuf *old = std::cerr.rdbuf(buffer.rdbuf());
auto screen = ScreenInteractive::FitComponent();
void UpdateScreen() {
  std::lock_guard<std::mutex> guard(ThreadLock);
  TerminalDisplay += buffer.str();
  std::stringstream().swap(buffer);
  TextDisplayBase::UpdateAll();
  screen.PostEvent(Event::Custom);
}

int main() {
  // Main Window
  Component MainDisplay = TextDisplay(DisplayMain, true) |
                          size(HEIGHT, GREATER_THAN, 20) |
                          size(WIDTH, GREATER_THAN, 60);
  MainDisplay = Wrap("Main Display", MainDisplay);

  // Terminal window
  Component TerminalOutput = TextDisplay(TerminalDisplay, true) |
                             size(HEIGHT, GREATER_THAN, 20) |
                             size(WIDTH, GREATER_THAN, 60);
  TerminalOutput = Wrap("Terminal", TerminalOutput);

  //
  //
  // WALLET SHTUFF  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Wallet Public Key Box
  std::string WalletPublicKey = "";
  auto WalletPublicKeyBox = ftxui::Input(&WalletPublicKey, "~");
  WalletPublicKeyBox = Wrap("Public Key", WalletPublicKeyBox);

  // Wallet Private Key Box
  std::string WalletPrivateKey = "";
  auto WalletPrivateKeyBox = ftxui::Input(&WalletPrivateKey, "~");
  WalletPrivateKeyBox = Wrap("Private Key", WalletPrivateKeyBox);

  // Generate Wallet Button
  auto GenerateWallet = ftxui::Button("Generate Wallet", [&] {
    LocalWallet.InstantiateWallet(PrivateKey = Wallet::Generate_Private_Key());
    WalletPublicKey = LocalWallet.PublicKey;
    WalletPrivateKey = PrivateKey;
    UpdateScreen();
  });

  // Verify Wallet Button
  auto VerifyWallet = ftxui::Button("Verify Wallet", [&] {
    if (Wallet::Verify_Key_Pair(WalletPrivateKey, WalletPublicKey)) {
      DisplayMain = "Keys are valid";
    } else {
      DisplayMain = "Keys are invalid";
    }
    UpdateScreen();
  });

  // Get Wallet ammount button
  auto DisplayWalletContents = ftxui::Button("Display Contents", [&] {
    DisplayMain =
        std::to_string(LocalChain.CheckNumericBalance(WalletPublicKey));
    UpdateScreen();
  });

  // Save and load Wallet Key Pair to File

  std::string SaveLoc = "Wallet.txt";
  auto SaveBox = ftxui::Input(&SaveLoc, "~");
  SaveBox = Wrap("Save File", SaveBox);

  auto SaveKeyPair = ftxui::Button("Save Key Pair",
                                   [&] {
                                     std::ofstream SaveFile(SaveLoc);
                                     SaveFile << PrivateKey << std::endl;
                                     SaveFile << LocalWallet.PublicKey;
                                   }) |
                     color(Color::Green);

  auto LoadKeyPair = ftxui::Button("Load Key Pair",
                                   [&] {
                                     std::ifstream SaveFile(SaveLoc);
                                     getline(SaveFile, PrivateKey);
                                     getline(SaveFile, LocalWallet.PublicKey);
                                     WalletPublicKey = LocalWallet.PublicKey;
                                     WalletPrivateKey = PrivateKey;
                                     UpdateScreen();
                                   }) |
                     color(Color::Blue);

  // Wallet Layout ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  auto WalletArea = Container::Vertical({
      WalletPublicKeyBox,
      WalletPrivateKeyBox,
      Container::Horizontal({
          GenerateWallet,
          VerifyWallet,
          DisplayWalletContents,
      }),
      Container::Horizontal({
          SaveBox,
          SaveKeyPair,
          LoadKeyPair,
      }),

  });
  WalletArea = Wrap("Wallet", WalletArea);

  //
  //
  // Funds Transfer Section ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Ammount Box
  std::string TransactionAmmount = "0";
  auto TransactionAmmountBox = ftxui::Input(&TransactionAmmount, "~");
  TransactionAmmountBox = Wrap("Ammount", TransactionAmmountBox);

  // Fee Box
  std::string TransactionFee = "0";
  auto TransactionFeeBox = ftxui::Input(&TransactionFee, "~");
  TransactionFeeBox = Wrap("Fee", TransactionFeeBox);

  // Reciever Box
  std::string Reciever = "";
  auto RecieverBox = ftxui::Input(&Reciever, "~");
  RecieverBox = Wrap("Reciever", RecieverBox);

  // Generate Transaction Button
  auto GenerateTransaction = ftxui::Button("Generate Transaction", [&] {
    CurrentTransaction.Init(PrivateKey, LocalWallet.PublicKey, Reciever,
                            std::stoi(TransactionAmmount),
                            std::stoi(TransactionFee));
    DisplayMain = CurrentTransaction.PRINT_TRANSACTION();
    LocalChain.PushTransaction(CurrentTransaction);
    UpdateScreen();
  });

  auto TransactionArea = Container::Vertical({
      Container::Horizontal({TransactionAmmountBox, TransactionFeeBox}),
      Container::Horizontal({RecieverBox, GenerateTransaction}),
  });
  TransactionArea = Wrap("Transaction", TransactionArea);

  //
  //
  // BLOCK STUFF ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Block Index Input
  std::string BlockIndex = "0";
  auto BlockIndexInput = ftxui::Input(&BlockIndex, "0");
  BlockIndexInput = Wrap("Block", BlockIndexInput);

  // Display Block Button
  auto PrintBlock = ftxui::Button("Print Index", [&] {
    try {
      DisplayMain = LocalChain.Get_Block(std::stoi(BlockIndex)).PrintBlock();
    } catch (...) {
      DisplayMain = "Block Not Found";
    }
    UpdateScreen();
  });

  auto PrintChainIndex = ftxui::Container::Horizontal({
      BlockIndexInput,
      PrintBlock,
  });

  auto NewBlock = ftxui::Button("New Block", [&] {
    LocalChain.NewBlock(LocalWallet.PublicKey);
    DisplayMain = LocalChain.Get_Last_Block().PrintBlock();
    UpdateScreen();
  });

  auto PrintChain = ftxui::Button("Print Chain", [&] {
    DisplayMain = LocalChain.PRINT_CHAIN();
    UpdateScreen();
  });

  auto PrintPool = ftxui::Button("Print Transaction Pool", [&] {
    DisplayMain = LocalChain.PRINT_POOL();
    UpdateScreen();
  });

  auto ValidateChain = ftxui::Button("Validate Chain", [&] {
    if (LocalChain.Validate()) {
      DisplayMain = "Chain Sucessfully Validated";
    } else {
      DisplayMain = "Invalid Chain";
    }
    UpdateScreen();
  });

  // Chain Layout
  auto ChainArea = Container::Vertical({
      PrintChainIndex,
      Container::Horizontal({
          NewBlock,
          PrintChain,
          PrintPool,
          ValidateChain,
      }),
  });

  ChainArea = Wrap("Chain", ChainArea);

  // Test Functions  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  auto RunTimings = ftxui::Button("Run Timings", [&] {
    std::cerr << "Running Tests" << "\n";
    DisplayMain = "";
    for (int i = 0; i < 6; i++) {
      Dificulty_Threashold = i;
      unsigned Nonce = 0;
      UpdateScreen();
      MULTITHREAD = false;
      auto Start = std::chrono::high_resolution_clock::now();
      std::string Data = Block::Mine("TestData", Nonce);
      auto End = std::chrono::high_resolution_clock::now();

      int NotThreaded =
          std::chrono::duration_cast<std::chrono::milliseconds>(End - Start)
              .count();

      DisplayMain +=
          "\n\nTiming Without threading: " + std::to_string(NotThreaded) +
          "ms" + "\nHash: " + StringToHex(Data) +
          "\nNonce: " + std::to_string(Nonce);
      UpdateScreen();

      Nonce = 0;
      MULTITHREAD = true;
      Start = std::chrono::high_resolution_clock::now();
      Data = Block::Mine("TestData", Nonce);
      End = std::chrono::high_resolution_clock::now();

      int Threaded =
          std::chrono::duration_cast<std::chrono::milliseconds>(End - Start)
              .count();
      DisplayMain += "\nTiming With threading: " + std::to_string(Threaded) +
                     "ms" + "\nHash: " + StringToHex(Data) +
                     "\nNonce: " + std::to_string(Nonce);
      UpdateScreen();
    }
    std::ofstream DD("DisplayData.txt");
    DD << DisplayMain << std::endl;
    DD.close();
  });

  auto TestArea = Container::Vertical({
      RunTimings,
  });
  TestArea = Wrap("Testing", TestArea);
  //
  //
  // Boiler plate main display  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  auto Display = Container::Horizontal({
                     Container::Vertical({
                         MainDisplay,
                         TerminalOutput,
                     }),
                     Container::Vertical({
                         WalletArea,
                         TransactionArea,
                         ChainArea,
                         TestArea,
                     }),
                 }) |
                 size(WIDTH, GREATER_THAN, 120);

  auto RenderLayout = Renderer(Display, [&] {
    return (hbox({
               Display->Render(),
           })) |
           border;
  });

  bool Run = true;
  std::thread Clock([&] {
    while (Run) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      UpdateScreen();
    }
  });

  screen.Loop(RenderLayout);
  return (0);
}
