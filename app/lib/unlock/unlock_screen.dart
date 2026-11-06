import 'package:flutter/material.dart';

import '../auth_method.dart';

class UnlockScreen extends StatelessWidget {
  const UnlockScreen({
    super.key,
    required this.enabledMethods,
    required this.completedMethods,
    required this.remainingSeconds,
    required this.onApprove,
    required this.onReset,
  });

  final Set<AuthMethod> enabledMethods;
  final Set<AuthMethod> completedMethods;
  final int remainingSeconds;
  final void Function(AuthMethod) onApprove;
  final VoidCallback onReset;

  void _openMethod(BuildContext context, AuthMethod method) {
    final approved = completedMethods.contains(method);

    showModalBottomSheet<void>(
      context: context,
      showDragHandle: true,
      builder: (sheetContext) => SafeArea(
        child: Padding(
          padding: const EdgeInsets.fromLTRB(24, 8, 24, 24),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Icon(
                method.icon,
                size: 40,
                color: Theme.of(context).colorScheme.primary,
              ),
              const SizedBox(height: 16),
              Text(
                method.label,
                style: Theme.of(context).textTheme.headlineSmall,
              ),
              const SizedBox(height: 8),
              Text(
                approved ? 'This step is approved for this demo.' : 'Demo step only · No verification or door command is sent.',
              ),
              const SizedBox(height: 24),
              FilledButton(
                onPressed: approved
                    ? null
                    : () {
                        onApprove(method);
                        Navigator.pop(sheetContext);
                      },
                child: Text(approved ? 'Approved' : 'Approve (demo)'),
              ),
            ],
          ),
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final methods = AuthMethod.values.where(enabledMethods.contains).toList();
    final completed = methods.where(completedMethods.contains).length;
    final total = methods.length;

    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Row(
          children: [
            Expanded(
              child: Text(
                'Unlock door',
                style: Theme.of(context).textTheme.headlineSmall,
              ),
            ),
            Icon(
              Icons.timer_outlined,
              color: Theme.of(context).colorScheme.primary,
            ),
            const SizedBox(width: 4),
            Text(
              '${remainingSeconds ~/ 60}:${(remainingSeconds % 60).toString().padLeft(2, '0')}',
              style: Theme.of(context).textTheme.titleMedium
                  ?.copyWith(color: Theme.of(context).colorScheme.primary),
            ),
          ],
        ),
        const SizedBox(height: 4),
        const Text('Frontend preview · No unlock command will be sent.'),
        const SizedBox(height: 24),
        Text(
          '$completed of $total steps complete',
          style: Theme.of(context).textTheme.titleMedium,
        ),
        const SizedBox(height: 12),
        LinearProgressIndicator(value: total == 0 ? 0 : completed / total),
        const SizedBox(height: 20),
        for (final method in methods)
          Card(
            child: ListTile(
              leading: Icon(
                method.icon,
                color: Theme.of(context).colorScheme.primary,
              ),
              title: Text(method.label),
              subtitle: Text(
                completedMethods.contains(method) ? 'Approved' : 'Still needed',
              ),
              trailing: Icon(
                completedMethods.contains(method)
                    ? Icons.check_circle
                    : Icons.arrow_forward_ios,
                color: Theme.of(context).colorScheme.primary,
              ),
              onTap: () => _openMethod(context, method),
            ),
          ),
        if (completed == total) ...[
          const SizedBox(height: 16),
          const Text('All steps approved for this demo. Door remains locked.'),
          TextButton(onPressed: onReset, child: const Text('Reset demo steps')),
        ],
      ],
    );
  }
}
