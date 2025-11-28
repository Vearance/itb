from typing import List, Dict
from datetime import datetime
from models.Transaction import Transaction
from models.Account import Account
from models.Target import Target
from database import Database
import csv
import json
try:
    from reportlab.lib.pagesizes import letter, A4
    from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
    from reportlab.lib.units import inch
    from reportlab.platypus import SimpleDocTemplate, Table, TableStyle, Paragraph, Spacer, PageBreak
    from reportlab.lib import colors
    REPORTLAB_AVAILABLE = True
except ImportError:
    REPORTLAB_AVAILABLE = False

class ReportController:   
    def __init__(self, db: Database = None):
        """Initialize ReportController"""
        self.db = db

    def generate_transaction_report(
        self,
        transactions: List[Transaction],
        account: Account
    ) -> Dict:
        """
        Args:
            transactions: List of transactions
            account: Associated account

        Returns:
            Dictionary containing report data
        """
        total_income = 0.0
        total_expense = 0.0
        transaction_count = 0
        transaction_details = []

        for transaction in transactions:
            if transaction.is_income():
                total_income += transaction.amount
            else:
                total_expense += transaction.amount
            transaction_count += 1
            
            # Add transaction detail
            transaction_details.append({
                'transaction_id': transaction.transaction_id,
                'date': transaction.date.isoformat() if transaction.date else '',
                'description': transaction.description,
                'amount': transaction.amount,
                'type': 'Income' if transaction.is_income() else 'Expense'
            })

        return {
            'account_id': account.account_id,
            'account_name': account.account_name,
            'current_balance': account.get_balance(),
            'total_income': total_income,
            'total_expense': total_expense,
            'net_change': total_income - total_expense,
            'transaction_count': transaction_count,
            'transactions': transaction_details,
            'generated_at': datetime.now().isoformat()
        }

    def generate_account_summary(
        self,
        account: Account,
        transactions: List[Transaction],
        targets: List[Target]
    ) -> Dict:
        """
        Args:
            account: Account to summarize
            transactions: Associated transactions
            targets: Associated targets 
        Returns:
            Dictionary containing summary data
        """
        transaction_report = self.generate_transaction_report(transactions, account)

        target_summary = []
        for target in targets:
            target_summary.append({
                'target_id': target.target_id,
                'target_name': target.target_name,
                'target_amount': target.target_amount,
                'current_amount': target.current_amount,
                'progress': target.get_percentage(),
                'is_achieved': target.is_achieved,
                'is_archived': target.is_archived,
                'deadline': target.deadline.isoformat()
            })

        return {
            'account_info': {
                'account_id': account.account_id,
                'account_name': account.account_name,
                'current_balance': account.get_balance()
            },
            'transaction_summary': transaction_report,
            'targets': target_summary,
            'generated_at': datetime.now().isoformat()
        }

    def export_report(
        self,
        report_data: Dict,
        filename: str,
        format: str = 'json'
    ) -> bool:
        """
        Args:
            report_data: Report data dictionary
            filename: Output filename
            format: Export format (json, csv, txt, pdf)
        Returns:
            bool: True if export successful
        """
        try:
            if format == 'json':

                with open(filename, 'w') as f:
                    json.dump(report_data, f, indent=2)
                return True
            elif format == 'csv':
                return self.export_to_csv(report_data, filename)
            elif format == 'txt':
                return self.export_to_txt(report_data, filename)
            elif format == 'pdf':
                return self.export_to_pdf(report_data, filename)
        except Exception as e:
            print(f"Error exporting report: {e}")
            return False

    def export_to_csv(self, report_data: Dict, filename: str) -> bool:
        """Export report to CSV format"""
        try:
            with open(filename, 'w', newline='') as f:
                writer = csv.writer(f)
                # Write account info
                writer.writerow(['Account Report'])
                writer.writerow(['Account ID', report_data['account_id']])
                writer.writerow(['Account Name', report_data['account_name']])
                writer.writerow(['Current Balance', f"Rp {report_data['current_balance']:,.2f}"])
                writer.writerow([])

                # Write transaction summary
                writer.writerow(['Transaction Summary'])
                writer.writerow(['Total Income', f"Rp {report_data['total_income']:,.2f}"])
                writer.writerow(['Total Expense', f"Rp {report_data['total_expense']:,.2f}"])
                writer.writerow(['Net Change', f"Rp {report_data['net_change']:,.2f}"])
                writer.writerow(['Total Transactions', report_data['transaction_count']])
                writer.writerow([])

                # Write detailed transactions
                if 'transactions' in report_data and report_data['transactions']:
                    writer.writerow(['Detailed Transactions'])
                    writer.writerow(['Date', 'Description', 'Type', 'Amount'])
                    for txn in report_data['transactions']:
                        writer.writerow([
                            txn['date'],
                            txn['description'],
                            txn['type'],
                            f"Rp {txn['amount']:,.2f}"
                        ])
                    writer.writerow([])

            return True
        except Exception as e:
            print(f"Error exporting to CSV: {e}")
            return False

    def export_to_txt(self, report_data: Dict, filename: str) -> bool:
        """Export report to TXT format"""
        try:
            with open(filename, 'w') as f:
                f.write("=" * 70 + "\n")
                f.write("TABUNGIN - Financial Report\n")
                f.write("=" * 70 + "\n\n")

                # Account Info
                f.write("ACCOUNT INFORMATION\n")
                f.write("-" * 70 + "\n")
                f.write(f"Account ID: {report_data['account_id']}\n")
                f.write(f"Account Name: {report_data['account_name']}\n")
                f.write(f"Current Balance: Rp {report_data['current_balance']:,.2f}\n\n")

                # Transaction Summary
                f.write("TRANSACTION SUMMARY\n")
                f.write("-" * 70 + "\n")
                f.write(f"Total Income: Rp {report_data['total_income']:,.2f}\n")
                f.write(f"Total Expense: Rp {report_data['total_expense']:,.2f}\n")
                f.write(f"Net Change: Rp {report_data['net_change']:,.2f}\n")
                f.write(f"Transaction Count: {report_data['transaction_count']}\n\n")

                # Detailed Transactions
                if 'transactions' in report_data and report_data['transactions']:
                    f.write("DETAILED TRANSACTIONS\n")
                    f.write("-" * 70 + "\n")
                    for txn in report_data['transactions']:
                        f.write(f"\nDate: {txn['date']}\n")
                        f.write(f"  Description: {txn['description']}\n")
                        f.write(f"  Type: {txn['type']}\n")
                        f.write(f"  Amount: Rp {txn['amount']:,.2f}\n")

                f.write("\n" + "=" * 70 + "\n")
                f.write(f"Report Generated: {report_data['generated_at']}\n")
                f.write("=" * 70 + "\n")
            return True
        except Exception as e:
            print(f"Error exporting to TXT: {e}")
            return False

    def export_to_pdf(self, report_data: Dict, filename: str) -> bool:
        """Export report to PDF format using ReportLab"""
        if not REPORTLAB_AVAILABLE:
            print("ReportLab not installed. Install with: pip install reportlab")
            return False
        
        try:
            # Create PDF document
            doc = SimpleDocTemplate(filename, pagesize=letter, topMargin=0.5 * inch, bottomMargin=0.5 * inch)
            styles = getSampleStyleSheet()
            story = []
            
            # Title
            title_style = ParagraphStyle(
                'CustomTitle',
                parent=styles['Heading1'],
                fontSize=24,
                textColor=colors.HexColor('#1a1a1a'),
                spaceAfter=30,
                alignment=1  # Center
            )
            story.append(Paragraph("TABUNGIN - Laporan Keuangan", title_style))
            story.append(Spacer(1, 0.3 * inch))
            
            # Account Info
            account_data = [
                ['Account ID:', report_data['account_id']],
                ['Account Name:', report_data['account_name']],
                ['Current Balance:', f"Rp {report_data['current_balance']:,.2f}"],
                ['Report Generated:', report_data['generated_at'][:10]],
            ]
            account_table = Table(account_data, colWidths=[2 * inch, 4 * inch])
            account_table.setStyle(TableStyle([
                ('BACKGROUND', (0, 0), (0, -1), colors.HexColor('#2d2d2d')),
                ('TEXTCOLOR', (0, 0), (0, -1), colors.whitesmoke),
                ('ALIGN', (0, 0), (-1, -1), 'LEFT'),
                ('FONTNAME', (0, 0), (0, -1), 'Helvetica-Bold'),
                ('FONTSIZE', (0, 0), (-1, -1), 10),
                ('BOTTOMPADDING', (0, 0), (-1, -1), 12),
                ('GRID', (0, 0), (-1, -1), 1, colors.HexColor('#666666'))
            ]))
            story.append(account_table)
            story.append(Spacer(1, 0.3 * inch))
            
            # Transaction Summary
            summary_data = [
                ['Metric', 'Amount'],
                ['Total Income', f"Rp {report_data['total_income']:,.2f}"],
                ['Total Expense', f"Rp {report_data['total_expense']:,.2f}"],
                ['Net Change', f"Rp {report_data['net_change']:,.2f}"],
                ['Transaction Count', str(report_data['transaction_count'])],
            ]
            summary_table = Table(summary_data, colWidths=[3 * inch, 3 * inch])
            summary_table.setStyle(TableStyle([
                ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor('#2d2d2d')),
                ('TEXTCOLOR', (0, 0), (-1, 0), colors.whitesmoke),
                ('ALIGN', (0, 0), (-1, -1), 'LEFT'),
                ('FONTNAME', (0, 0), (-1, 0), 'Helvetica-Bold'),
                ('FONTSIZE', (0, 0), (-1, -1), 10),
                ('BOTTOMPADDING', (0, 0), (-1, -1), 12),
                ('GRID', (0, 0), (-1, -1), 1, colors.HexColor('#666666')),
                ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, colors.HexColor('#f5f5f5')])
            ]))
            story.append(Paragraph("Transaction Summary", styles['Heading2']))
            story.append(summary_table)
            story.append(Spacer(1, 0.3 * inch))
            
            # Detailed Transactions
            if 'transactions' in report_data and report_data['transactions']:
                story.append(Paragraph("Detailed Transactions", styles['Heading2']))
                
                trans_data = [['Date', 'Description', 'Type', 'Amount']]
                for txn in report_data['transactions']:
                    trans_data.append([
                        txn['date'][:10],
                        txn['description'][:30],  # Truncate long descriptions
                        txn['type'],
                        f"Rp {txn['amount']:,.0f}"
                    ])
                
                trans_table = Table(trans_data, colWidths=[1.2 * inch, 2.5 * inch, 1 * inch, 1.3 * inch])
                trans_table.setStyle(TableStyle([
                    ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor('#2d2d2d')),
                    ('TEXTCOLOR', (0, 0), (-1, 0), colors.whitesmoke),
                    ('ALIGN', (0, 0), (-1, -1), 'LEFT'),
                    ('ALIGN', (3, 0), (3, -1), 'RIGHT'),
                    ('FONTNAME', (0, 0), (-1, 0), 'Helvetica-Bold'),
                    ('FONTSIZE', (0, 0), (-1, -1), 9),
                    ('BOTTOMPADDING', (0, 0), (-1, -1), 10),
                    ('GRID', (0, 0), (-1, -1), 1, colors.HexColor('#666666')),
                    ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, colors.HexColor('#f5f5f5')])
                ]))
                story.append(trans_table)
            
            # Build PDF
            doc.build(story)
            return True
        except Exception as e:
            print(f"Error exporting to PDF: {e}")
            return False

# TODO: add pdf export
