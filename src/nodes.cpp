#include "nodes.hpp"

Storehouse::Storehouse(std::unique_ptr<IPackageStockpile> stockpile) : stockpile_(std::move(stockpile)){}

void Storehouse::ReceivePackage(Package&& package){stockpile_->Push(std::move(package));}

Package Storehouse::ReleasePackage(){return stockpile_->Pop();}

bool Storehouse::Empty() const{return stockpile_-> Empty();}