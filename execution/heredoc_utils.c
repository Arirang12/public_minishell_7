/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbouchri <mbouchri@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/22 10:56:07 by mbouchri          #+#    #+#             */
/*   Updated: 2025/08/24 20:30:38 by mbouchri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "execution.h"

char	*heredoc_name(void)
{
	static int		counter;
	char	*num;
	char	*file;

	num = ft_itoa(counter++);
	file = ft_strjoin("/tmp/ms_heredoc_", num);
	free(num);
	return (file);
}

int	heredoc_fd_new(char **path)
{
	int	fd;

	//if (!*path)
	//	return (-1);
	while (1)
	{
		*path = heredoc_name();
		//unlink(*path);
		if (!*path)
			return (-1);
		fd = open(*path, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd != -1)
			break ;
		//unlink(*path);
		free(*path);
		//*path = heredoc_name();
	}
	return (fd);
}

int	fork_heredoc(int fd, char *limiter, t_cmd *cmd, t_env *env, bool expand, char *path)
{
	pid_t	pid;

	pid = fork();
	if (pid < 0)
		return (-1);
	if (pid == 0)
	{
		signal(SIGINT, sigint_heredoc);
		signal(SIGQUIT, SIG_IGN);
		heredoc_input(fd, limiter, env, expand);
		//free(limiter); // free 1
		(void)cmd;
		free_all(cmd, env);
		close(fd);
		free(path);
		//write(STDERR_FILENO, "*-------------end!--------------*", 34);
		exit(0);
	}
	close(fd);
	return (pid);
}

char	*wait_heredoc(pid_t pid, char *path)
{
	int	status;

	if (waitpid(pid, &status, 0) == -1 || status != 0)
	{
		unlink(path);
		free(path);
		g_exit = 130;
		return (NULL);
	}
	return (path);
}

char	*make_heredoc(char *limiter,t_cmd *cmd, t_env *env, bool expand)
{
	char	*path;
	int		fd;
	int		pid;

	fd = heredoc_fd_new(&path);
	if (fd == -1)
		return (NULL);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	pid = fork_heredoc(fd, limiter, cmd, env, expand, path);
	if (pid == -1)
	{
		unlink(path);
		free(path);
		signal(SIGINT, sigint_prompt);
		signal(SIGQUIT, SIG_IGN);
		return (NULL);
	}
	path = wait_heredoc(pid, path);
	signal(SIGINT, sigint_prompt);
	signal(SIGQUIT, SIG_IGN);
	return (path);
}
