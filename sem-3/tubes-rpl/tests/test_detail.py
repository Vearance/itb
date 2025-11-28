from PyQt6.QtWidgets import QApplication
from models.User import User
from views.LaporanView import DetailView
from datetime import datetime
import sys


def test_detail_mingguan():
    """Test DetailView untuk laporan mingguan"""
    app = QApplication(sys.argv)
    
    user = User("user_001", "Rafi Akbar", "password123")
    
    # Data laporan mingguan - 24 November - 30 November 2025
    data_mingguan = {
        "filter_type": "Mingguan",
        "start_date": datetime(2025, 11, 24),
        "end_date": datetime(2025, 11, 30),
        "accounts": [
            {
                "account_name": "Gopay",
                "account_id": "82331241249174123",
                "income": 1500000,
                "expense": 600000,
                "admin_fee": 7500,
                "total": 892500
            },
            {
                "account_name": "Dana",
                "account_id": "82332241249174123",
                "income": 1200000,
                "expense": 450000,
                "admin_fee": 5000,
                "total": 745000
            },
            {
                "account_name": "BCA Mobile",
                "account_id": "82333241249174123",
                "income": 2000000,
                "expense": 800000,
                "admin_fee": 10000,
                "total": 1190000
            }
        ],
        
        "transactions": [
            {"description": "Gaji freelance", "date": datetime(2025, 11, 25, 9, 30), "amount": 2000000, "type": "income", "wallet": "BCA Mobile"},
            {"description": "Bayar makan siang", "date": datetime(2025, 11, 24, 15, 0), "amount": 60000, "type": "expense", "wallet": "Gopay"},
            {"description": "Isi bensin", "date": datetime(2025, 11, 24, 10, 0), "amount": 100000, "type": "expense", "wallet": "Dana"},
            {"description": "Transfer dari Ibu", "date": datetime(2025, 11, 27, 11, 0), "amount": 500000, "type": "income", "wallet": "Gopay"},
            {"description": "Beli buku", "date": datetime(2025, 11, 28, 11, 0), "amount": 200000, "type": "expense", "wallet": "BCA Mobile"},
            {"description": "Penjualan Barang", "date": datetime(2025, 11, 29, 11, 0), "amount": 700000, "type": "income", "wallet": "Dana"},
        ]
    }
    
    detail_view = DetailView(user, "report", data_mingguan)
    
    # Connect signals
    detail_view.back_requested.connect(lambda: print("✓ Back to history"))
    detail_view.report_export_requested.connect(lambda fmt: print(f"✓ Export as {fmt}"))
    
    detail_view.show()
    
    print("=" * 50)
    print("Test: Laporan Mingguan")
    print("=" * 50)
    print(f"Range: {data_mingguan['start_date'].strftime('%d %B')} - {data_mingguan['end_date'].strftime('%d %B %Y')}")
    print(f"Jumlah Rekening: {len(data_mingguan['accounts'])}")
    total_income = sum(acc['income'] for acc in data_mingguan['accounts'])
    total_expense = sum(acc['expense'] for acc in data_mingguan['accounts'])
    total_admin = sum(acc['admin_fee'] for acc in data_mingguan['accounts'])
    print(f"Total Pendapatan: Rp{total_income:,}")
    print(f"Total Pengeluaran: Rp{total_expense:,}")
    print(f"Total Biaya Admin: Rp{total_admin:,}")
    print(f"Total Bersih: Rp{total_income - total_expense - total_admin:,}")
    print("=" * 50)
    
    sys.exit(app.exec())


if __name__ == "__main__":
    test_detail_mingguan()



# """
# =================================================================
# FILE 1: test_detail_transaction.py
# Test untuk transaksi individual
# =================================================================
# """
# from PyQt6.QtWidgets import QApplication
# from models.User import User
# from views.DetailView import DetailView
# from datetime import datetime
# import sys


# def test_detail_transaction():
#     """Test DetailView untuk transaksi individual"""
#     app = QApplication(sys.argv)
    
#     user = User("user_001", "Rafi Akbar", "password123")
    
#     # Data transaksi individual - PENDAPATAN
#     data_income = {
#         "account_name": "Gopay",
#         "account_id": "82331241249174123",
#         "amount": 200000,
#         "admin_fee": 2500,
#         "total": 202500,
#         "type": "income",
#         "date": datetime(2025, 11, 24),
#         "status": True
#     }
    
#     detail_view = DetailView(user, "transaction", data_income)
    
#     # Connect signals
#     detail_view.back_requested.connect(lambda: print("✓ Back to history"))
#     detail_view.report_export_requested.connect(lambda fmt: print(f"✓ Export as {fmt}"))
    
#     detail_view.show()
    
#     print("=" * 50)
#     print("Test: Detail Transaksi Individual (PENDAPATAN)")
#     print("=" * 50)
#     print(f"Account: {data_income['account_name']}")
#     print(f"Type: {data_income['type']}")
#     print(f"Amount: Rp{data_income['amount']:,}")
#     print(f"Date: {data_income['date'].strftime('%A, %d %B %Y')}")
#     print("=" * 50)
    
#     sys.exit(app.exec())


# if __name__ == "__main__":
#     test_detail_transaction()


# """
# =================================================================
# FILE 2: test_detail_harian.py
# Test untuk laporan harian
# =================================================================
# """
# from PyQt6.QtWidgets import QApplication
# from models.User import User
# from views.DetailView import DetailView
# from datetime import datetime
# import sys


# def test_detail_harian():
#     """Test DetailView untuk laporan harian"""
#     app = QApplication(sys.argv)
    
#     user = User("user_001", "Rafi Akbar", "password123")
    
#     # Data laporan harian - Senin, 24 November 2025
#     data_harian = {
#         "filter_type": "Harian",
#         "start_date": datetime(2025, 11, 24),
#         "end_date": datetime(2025, 11, 24),
#         "accounts": [
#             {
#                 "account_name": "Gopay",
#                 "account_id": "82331241249174123",
#                 "income": 200000,
#                 "expense": 50000,
#                 "admin_fee": 0,
#                 "total": 150000
#             },
#             {
#                 "account_name": "Dana",
#                 "account_id": "82332241249174123",
#                 "income": 150000,
#                 "expense": 30000,
#                 "admin_fee": 0,
#                 "total": 120000
#             },
#             {
#                 "account_name": "BCA Mobile",
#                 "account_id": "82333241249174123",
#                 "income": 300000,
#                 "expense": 100000,
#                 "admin_fee": 2500,
#                 "total": 197500
#             }
#         ]
#     }
    
#     detail_view = DetailView(user, "report", data_harian)
    
#     # Connect signals
#     detail_view.back_requested.connect(lambda: print("✓ Back to history"))
#     detail_view.report_export_requested.connect(lambda fmt: print(f"✓ Export as {fmt}"))
    
#     detail_view.show()
    
#     print("=" * 50)
#     print("Test: Laporan Harian")
#     print("=" * 50)
#     print(f"Tanggal: {data_harian['start_date'].strftime('%A, %d %B %Y')}")
#     print(f"Jumlah Rekening: {len(data_harian['accounts'])}")
#     total_income = sum(acc['income'] for acc in data_harian['accounts'])
#     total_expense = sum(acc['expense'] for acc in data_harian['accounts'])
#     print(f"Total Pendapatan: Rp{total_income:,}")
#     print(f"Total Pengeluaran: Rp{total_expense:,}")
#     print(f"Total Bersih: Rp{total_income - total_expense:,}")
#     print("=" * 50)
    
#     sys.exit(app.exec())


# if __name__ == "__main__":
#     test_detail_harian()


# """
# =================================================================
# FILE 3: test_detail_bulanan.py
# Test untuk laporan bulanan
# =================================================================
# """
# from PyQt6.QtWidgets import QApplication
# from models.User import User
# from views.DetailView import DetailView
# from datetime import datetime
# import sys


# def test_detail_bulanan():
#     """Test DetailView untuk laporan bulanan"""
#     app = QApplication(sys.argv)
    
#     user = User("user_001", "Rafi Akbar", "password123")
    
#     # Data laporan bulanan - November 2025
#     data_bulanan = {
#         "filter_type": "Bulanan",
#         "start_date": datetime(2025, 11, 1),
#         "end_date": datetime(2025, 11, 30),
#         "accounts": [
#             {
#                 "account_name": "Gopay",
#                 "account_id": "82331241249174123",
#                 "income": 5000000,
#                 "expense": 2000000,
#                 "admin_fee": 15000,
#                 "total": 2985000
#             },
#             {
#                 "account_name": "Dana",
#                 "account_id": "82332241249174123",
#                 "income": 3500000,
#                 "expense": 1500000,
#                 "admin_fee": 10000,
#                 "total": 1990000
#             },
#             {
#                 "account_name": "BCA Mobile",
#                 "account_id": "82333241249174123",
#                 "income": 8000000,
#                 "expense": 3000000,
#                 "admin_fee": 25000,
#                 "total": 4975000
#             }
#         ]
#     }
    
#     detail_view = DetailView(user, "report", data_bulanan)
    
#     # Connect signals
#     detail_view.back_requested.connect(lambda: print("✓ Back to history"))
#     detail_view.report_export_requested.connect(lambda fmt: print(f"✓ Export as {fmt}"))
    
#     detail_view.show()
    
#     print("=" * 50)
#     print("Test: Laporan Bulanan")
#     print("=" * 50)
#     print(f"Bulan: {data_bulanan['start_date'].strftime('%B %Y')}")
#     print(f"Jumlah Rekening: {len(data_bulanan['accounts'])}")
#     total_income = sum(acc['income'] for acc in data_bulanan['accounts'])
#     total_expense = sum(acc['expense'] for acc in data_bulanan['accounts'])
#     total_admin = sum(acc['admin_fee'] for acc in data_bulanan['accounts'])
#     print(f"Total Pendapatan: Rp{total_income:,}")
#     print(f"Total Pengeluaran: Rp{total_expense:,}")
#     print(f"Total Biaya Admin: Rp{total_admin:,}")
#     print(f"Total Bersih: Rp{total_income - total_expense - total_admin:,}")
#     print("=" * 50)
    
#     sys.exit(app.exec())


# if __name__ == "__main__":
#     test_detail_bulanan()


# """
# =================================================================
# FILE 4: test_detail_pilih_tanggal.py
# Test untuk laporan pilih tanggal (custom range)
# =================================================================
# """
# from PyQt6.QtWidgets import QApplication
# from models.User import User
# from views.DetailView import DetailView
# from datetime import datetime
# import sys


# def test_detail_pilih_tanggal_same_month():
#     """Test DetailView untuk laporan pilih tanggal (bulan sama)"""
#     app = QApplication(sys.argv)
    
#     user = User("user_001", "Rafi Akbar", "password123")
    
#     # Data laporan pilih tanggal - 24-26 November 2025 (same month)
#     data = {
#         "filter_type": "Pilih Tanggal",
#         "start_date": datetime(2025, 11, 24),
#         "end_date": datetime(2025, 11, 26),
#         "accounts": [
#             {
#                 "account_name": "Gopay",
#                 "account_id": "82331241249174123",
#                 "income": 500000,
#                 "expense": 200000,
#                 "admin_fee": 5000,
#                 "total": 295000
#             },
#             {
#                 "account_name": "Dana",
#                 "account_id": "82332241249174123",
#                 "income": 350000,
#                 "expense": 150000,
#                 "admin_fee": 0,
#                 "total": 200000
#             }
#         ]
#     }
    
#     detail_view = DetailView(user, "report", data)
    
#     # Connect signals
#     detail_view.back_requested.connect(lambda: print("✓ Back to history"))
#     detail_view.report_export_requested.connect(lambda fmt: print(f"✓ Export as {fmt}"))
    
#     detail_view.show()
    
#     print("=" * 50)
#     print("Test: Laporan Pilih Tanggal (Same Month)")
#     print("=" * 50)
#     print(f"Range: 24 - 26 November 2025")
#     print(f"Jumlah Rekening: {len(data['accounts'])}")
#     total_income = sum(acc['income'] for acc in data['accounts'])
#     total_expense = sum(acc['expense'] for acc in data['accounts'])
#     print(f"Total Pendapatan: Rp{total_income:,}")
#     print(f"Total Pengeluaran: Rp{total_expense:,}")
#     print("=" * 50)
    
#     sys.exit(app.exec())


# def test_detail_pilih_tanggal_diff_month():
#     """Test DetailView untuk laporan pilih tanggal (beda bulan)"""
#     app = QApplication(sys.argv)
    
#     user = User("user_001", "Rafi Akbar", "password123")
    
#     # Data laporan pilih tanggal - 24 November - 5 Desember 2025 (cross month)
#     data = {
#         "filter_type": "Pilih Tanggal",
#         "start_date": datetime(2025, 11, 24),
#         "end_date": datetime(2025, 12, 5),
#         "accounts": [
#             {
#                 "account_name": "Gopay",
#                 "account_id": "82331241249174123",
#                 "income": 1200000,
#                 "expense": 500000,
#                 "admin_fee": 15000,
#                 "total": 685000
#             },
#             {
#                 "account_name": "Dana",
#                 "account_id": "82332241249174123",
#                 "income": 800000,
#                 "expense": 300000,
#                 "admin_fee": 0,
#                 "total": 500000
#             },
#             {
#                 "account_name": "BCA Mobile",
#                 "account_id": "82333241249174123",
#                 "income": 2000000,
#                 "expense": 800000,
#                 "admin_fee": 20000,
#                 "total": 1180000
#             }
#         ]
#     }
    
#     detail_view = DetailView(user, "report", data)
    
#     # Connect signals
#     detail_view.back_requested.connect(lambda: print("✓ Back to history"))
#     detail_view.report_export_requested.connect(lambda fmt: print(f"✓ Export as {fmt}"))
    
#     detail_view.show()
    
#     print("=" * 50)
#     print("Test: Laporan Pilih Tanggal (Different Month)")
#     print("=" * 50)
#     print(f"Range: 24 November - 5 Desember 2025")
#     print(f"Jumlah Rekening: {len(data['accounts'])}")
#     total_income = sum(acc['income'] for acc in data['accounts'])
#     total_expense = sum(acc['expense'] for acc in data['accounts'])
#     print(f"Total Pendapatan: Rp{total_income:,}")
#     print(f"Total Pengeluaran: Rp{total_expense:,}")
#     print("=" * 50)
    
#     sys.exit(app.exec())


# if __name__ == "__main__":
#     # Pilih salah satu:
#     test_detail_pilih_tanggal_same_month()
#     # test_detail_pilih_tanggal_diff_month()