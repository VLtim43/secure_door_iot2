import 'package:flutter/material.dart';

class LogsScreen extends StatelessWidget {
  const LogsScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Text('Access log', style: Theme.of(context).textTheme.headlineSmall),
        const SizedBox(height: 4),
        const Text('Sample entries · Backend not connected'),
        const SizedBox(height: 16),
        const _LogEntry(
          time: 'Today · 09:41',
          method: 'Phone biometric · Admin',
          allowed: true,
        ),
        const _LogEntry(
          time: 'Yesterday · 18:22',
          method: 'PIN · Incorrect code',
          allowed: false,
        ),
        const _LogEntry(
          time: 'Yesterday · 08:15',
          method: 'TOTP · Admin',
          allowed: true,
        ),
      ],
    );
  }
}

class _LogEntry extends StatelessWidget {
  const _LogEntry({
    required this.time,
    required this.method,
    required this.allowed,
  });

  final String time;
  final String method;
  final bool allowed;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: ListTile(
        leading: Icon(
          allowed ? Icons.check_circle_outline : Icons.block,
          color: allowed ? Colors.green : Theme.of(context).colorScheme.error,
        ),
        title: Text(method),
        subtitle: Text(time),
        trailing: Text(allowed ? 'Allowed' : 'Denied'),
      ),
    );
  }
}
