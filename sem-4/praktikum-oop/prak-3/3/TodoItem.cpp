#include "TodoItem.hpp"
using namespace std;

TodoItem::TodoItem(const string& name) : Activity(name), isDone(false) {}

int TodoItem::complete() {
	if (!isDone) {
		isDone = true;
		return 10;
	}
	return 0;
}

string TodoItem::getStatus() const {
	return "[TODO] " + name + " - " + (isDone ? "Selesai" : "Belum");
}

TodoItem::~TodoItem() {
	cout << "Menghapus TodoItem " << name << "\n";
}